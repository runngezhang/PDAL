/******************************************************************************
* Copyright (c) 2018, Howard Butler, howard@hobu.co
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

#include "NumpyReader.hpp"

#include <pdal/PointView.hpp>
#include <pdal/pdal_features.hpp>
#include <pdal/PDALUtils.hpp>
#include <pdal/pdal_types.hpp>
#include <pdal/util/ProgramArgs.hpp>
#include <pdal/util/Algorithm.hpp>


namespace pdal
{

static PluginInfo const s_info = PluginInfo(
    "readers.numpy",
    "Read data from .npy files.",
    "" );

CREATE_SHARED_PLUGIN(1, 0, NumpyReader, Reader, s_info)

std::string NumpyReader::getName() const { return s_info.name; }


PyArrayObject* load_npy(std::string const& filename)
{

    PyObject* py_filename =  PyString_FromString(filename.c_str());
    PyObject* numpy_module = PyImport_ImportModule("numpy");
    if (!numpy_module)
        throw pdal::pdal_error(plang::getTraceback());

    PyObject* numpy_mod_dict = PyModule_GetDict(numpy_module);
    if (!numpy_mod_dict)
        throw pdal::pdal_error(plang::getTraceback());

    PyObject* loads_func = PyDict_GetItemString(numpy_mod_dict, "load");
    if (!loads_func)
        throw pdal::pdal_error(plang::getTraceback());

    PyObject* numpy_args = PyTuple_New(1);
    if (!numpy_args)
        throw pdal::pdal_error(plang::getTraceback());

    int success = PyTuple_SetItem(numpy_args, 0, py_filename);
    if (success != 0)
        throw pdal::pdal_error(plang::getTraceback());

    PyObject* array = PyObject_CallObject(loads_func, numpy_args);
    if (!array)
        throw pdal::pdal_error(plang::getTraceback());

    PyArrayObject* nparray = (PyArrayObject*)array;
    return nparray;
}


void NumpyReader::initialize()
{
    plang::Environment::get();
    m_index = 0;
    m_numPoints = 0;
    m_numDimensions = 0;

    m_array = load_npy(m_filename);
    if (!PyArray_Check(m_array))
        throw pdal::pdal_error("Object in file  '" + m_filename +
            "' is not a numpy array");
}

void NumpyReader::wakeUpNumpyArray()
{
    // TODO pivot whether we are a 1d, 2d, or named arrays

    if (PyArray_SIZE(m_array) == 0)
        throw pdal::pdal_error("Array cannot be 0!");

    m_iter = NpyIter_New(m_array, NPY_ITER_READONLY|
                             NPY_ITER_EXTERNAL_LOOP|
                             NPY_ITER_REFS_OK,
                         NPY_KEEPORDER, NPY_NO_CASTING,
                         NULL);
    if (m_iter == NULL)
    {
        throw pdal::pdal_error("Unable to create iterator from array in '" + m_filename +"'");
    }

    char* itererr;
    m_iternext = NpyIter_GetIterNext(m_iter, &itererr);

    if (m_iternext == NULL) {
        NpyIter_Deallocate(m_iter);
        throw pdal::pdal_error(itererr);
    }

    m_dtype = PyArray_DTYPE(m_array);
    if (!m_dtype)
        throw pdal::pdal_error(plang::getTraceback());


    int ndims = PyArray_NDIM(m_array);
    npy_intp* shape = PyArray_SHAPE(m_array);
    if (!shape)
        throw pdal::pdal_error(plang::getTraceback());

    // Get length of fields
    Py_ssize_t count = PyDict_Size(m_dtype->fields);

    // if there's only 1 ndims, but more than 1 count of them,
    // the data are arranged as named columns all of the same length
    // which is shape[0]
    if (ndims == 1)
    {
        // Named arrays, one at a time
        m_numPoints = shape[0];
        m_numDimensions = count;
    }
    log()->get(LogLevel::Debug) << "Adding " << count <<" dimensions" << std::endl;
    std::cerr<< "Adding " << count <<" dimensions" << std::endl;
    std::cerr<< "ndims " << ndims <<" dimensions" << std::endl;
    std::cerr<< "shape " << shape[0] <<" "  << std::endl;

}

void NumpyReader::addArgs(ProgramArgs& args)
{
}



void NumpyReader::addDimensions(PointLayoutPtr layout)
{
    using namespace Dimension;




    wakeUpNumpyArray();

    PyObject* names_dict = m_dtype->fields;
    PyObject* keys = PyDict_Keys(names_dict);
    if (!keys)
        throw pdal::pdal_error(plang::getTraceback());
    for (int i = 0; i < m_numDimensions; ++i)
    {
        // Get the dimension name
        PyObject* item = PyList_GetItem(keys, i);
        if (!item)
            throw pdal::pdal_error(plang::getTraceback());
        const char* cname = PyString_AsString(item);
        std::string name(cname);


        // Try sanitizing names to remove characters that would
        // make them valid PDAL dimension names otherwise
        std::string dash_s(name);
        std::string space_s(name);
        std::string under_s(name);
        pdal::Utils::remove(dash_s,'-');
        pdal::Utils::remove(space_s,' ');
        pdal::Utils::remove(under_s,'_');

        pdal::Dimension::Id dash = pdal::Dimension::id(dash_s);
        pdal::Dimension::Id space = pdal::Dimension::id(space_s);
        pdal::Dimension::Id under = pdal::Dimension::id(under_s);

        pdal::Dimension::Id id = pdal::Dimension::id(name);
        if (dash != pdal::Dimension::Id::Unknown)
            id = dash;
        else if (space != pdal::Dimension::Id::Unknown)
            id = space;
        else if (under != pdal::Dimension::Id::Unknown)
            id = under;

        pdal::Dimension::Type p_type = plang::Environment::getPDALDataType(m_dtype->type_num);
        m_types[id] = p_type;
        m_ids[id] = i;
        if (id == pdal::Dimension::Id::Unknown)
            layout->registerOrAssignDim(name, p_type);
        else
            layout->registerOrAssignDim(pdal::Dimension::name(id), p_type);

        std::cerr << "found known dimension "
                  << " '" << pdal::Dimension::name(id)
                  <<" 'for given name '"
                  <<" 'ptype: '" << m_dtype->type_num
                  <<" 'elsize: '" << m_dtype->elsize<< "' name '"
                  << name <<"'"
                  << "for type '" << pdal::Dimension::interpretationName(p_type) << "'"<< std::endl;

    }

}


void NumpyReader::ready(PointTableRef table)
{
    // wake up python
    static_cast<plang::Environment*>(plang::Environment::get())->set_stdout(log()->getLogStream());
    log()->get(LogLevel::Debug) << "Initializing Numpy array for file '" << m_filename <<"'" << std::endl;

    // for each dimension in m_ids_map, get the value from
    // the m_array Numpy array and set it on the point

    npy_intp* strideptr,* innersizeptr;

    // Set our iterators

    /* The location of the data pointer which the iterator may update */
    m_dataptr = NpyIter_GetDataPtrArray(m_iter);
    /* The location of the stride which the iterator may update */
    m_strideptr = NpyIter_GetInnerStrideArray(m_iter);
    /* The location of the inner loop size which the iterator may update */
    m_innersizeptr = NpyIter_GetInnerLoopSizePtr(m_iter);

}

void NumpyReader::loadPoint(PointRef& point, point_count_t position )
{


    char* data = *m_dataptr;
    npy_intp stride = *m_strideptr;
    npy_intp count = *m_innersizeptr;

    for (auto& i: m_ids)
    {
        pdal::Dimension::Id id = i.first;
        npy_intp idx = i.second;

        point.setField(id, m_types[id], (void *)data);
        data += stride;

        m_iternext(m_iter);
    }

}


bool NumpyReader::processOne(PointRef& point)
{
    if (m_index >= getNumPoints())
        return false;

    loadPoint(point, m_index);
    m_index += 1;

    return true;

}

point_count_t NumpyReader::getNumPoints() const
{
    if (m_array)
        return (point_count_t) m_numPoints;
    else
        throw pdal::pdal_error("Numpy array not initialized!");
}

point_count_t NumpyReader::read(PointViewPtr view, point_count_t count)
{

    PointId idx = view->size();
    point_count_t cnt = 0;
    while (cnt < count)
    {
        PointRef point(*view, idx);
        point.setPointId(idx);
        if (!processOne(point))
            break;
        cnt++;
        idx++;
    }
    return cnt;
}

void NumpyReader::done(PointTableRef)
{

    if (m_iter)
        NpyIter_Deallocate(m_iter);
}


} // namespace pdal