//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2021, Thierry Lelegard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------

#include "tsPSIPlugin.h"
#include "tsPluginRepository.h"
#include "tsPluginEventData.h"

TS_REGISTER_PROCESSOR_PLUGIN(u"psi", ts::PSIPlugin);

// A dummy storage value to force inclusion of this module when using the static library.
const int ts::PSIPlugin::REFERENCE = 0;


//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

ts::PSIPlugin::PSIPlugin(TSP* tsp_) :
    ProcessorPlugin(tsp_, u"Extract PSI Information", u"[options]"),
    _display(duck),
    _logger(_display),
    _signal_event(false),
    _event_code(0)
{
    duck.defineArgsForCAS(*this);
    duck.defineArgsForPDS(*this);
    duck.defineArgsForStandards(*this);
    duck.defineArgsForTimeReference(*this);
    duck.defineArgsForCharset(*this);
    _logger.defineArgs(*this);
    _display.defineArgs(*this);

    option(u"event-code", 0, UINT32);
    help(u"event-code",
         u"This option is for C++, Java or Python developers only.\n\n"
         u"Signal a plugin event with the specified code for each section. "
         u"The event data is an instance of PluginEventData pointing to the section content.");
}


//----------------------------------------------------------------------------
// Start / stop methods
//----------------------------------------------------------------------------

bool ts::PSIPlugin::getOptions()
{
    _signal_event = present(u"event-code");
    getIntValue(_event_code, u"event-code");
    _logger.setSectionHandler(_signal_event ? this : nullptr);
    return duck.loadArgs(*this) && _logger.loadArgs(duck, *this) && _display.loadArgs(duck, *this);
}


bool ts::PSIPlugin::start()
{
    return _logger.open();
}

bool ts::PSIPlugin::stop()
{
    _logger.close();
    return true;
}


//----------------------------------------------------------------------------
// Called by the TablesLogger for each section
//----------------------------------------------------------------------------

void ts::PSIPlugin::handleSection(SectionDemux&, const Section& sect)
{
    // Signal application-defined event. The call to the application callbacks is synchronous.
    if (_signal_event) {
        PluginEventData data(sect.content(), sect.size());
        tsp->signalPluginEvent(_event_code, &data);
    }
}


//----------------------------------------------------------------------------
// Packet processing method
//----------------------------------------------------------------------------

ts::ProcessorPlugin::Status ts::PSIPlugin::processPacket(TSPacket& pkt, TSPacketMetadata& pkt_data)
{
    _logger.feedPacket(pkt);
    return _logger.completed() ? TSP_END : TSP_OK;
}
