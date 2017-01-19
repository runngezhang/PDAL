/******************************************************************************
* Copyright (c) 2017, Howard Butler (howard@hobu.co)
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#pragma once

#include <pdal/Reader.hpp>
#include <pdal/util/Bounds.hpp>
#include <pdal/plugin.hpp>

#include "mbutil.hpp"


extern "C" int32_t MBSystemReader_ExitFunc();
extern "C" PF_ExitFunc MBSystemReader_InitPlugin();


namespace pdal
{


// The MrSIDReader wraps LT's PointSource abstraction
//
class PDAL_DLL MBSystemReader : public pdal::Reader
{

public:
    virtual ~MBSystemReader(){};
    MBSystemReader();
    MBSystemReader& operator=(const MBSystemReader&) = delete;
    MBSystemReader(const MBSystemReader&) = delete;

    static void * create();
    static int32_t destroy(void *);
    std::string getName() const;

    point_count_t getNumPoints() const
        {
            return 0;
        }

protected:
    virtual void addDimensions(PointLayoutPtr layout);

private:
    PointLayoutPtr m_layout;

    point_count_t m_index;

    virtual void initialize();
    virtual QuickInfo inspect();
    virtual void addArgs(ProgramArgs& args);
    virtual void ready(PointTableRef table)
        { ready(table, m_metadata); }
    virtual void ready(PointTableRef table, MetadataNode& m);
    virtual point_count_t read(PointViewPtr view, point_count_t count);
    virtual void done(PointTableRef table);
    virtual bool eof()
        { return m_index >= getNumPoints(); }
    bool m_initialized;

    struct mb_io_struct* mb_io;
};


} // namespaces
