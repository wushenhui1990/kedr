/// mist_engine.c
/// Implementation of MiST engine public API

/////////////////////////////////////////////////////////////////////////////
// Copyright 2009-2010 
// Institute for System Programming of the Russian Academy of Sciences (ISPRAS)
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//    http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License. 
/////////////////////////////////////////////////////////////////////////////

// config.h may be generated by 'configure' script. 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <assert.h>

#include "mist_engine.h"
#include "grar.h"
#include "smap.h"
#include "mist_base.h"

// API version requested by the application using the library.
// 0 means that the library has not been initialized yet.
static unsigned int mist_api_version = 0; 

EMistErrorCode
mist_engine_init(unsigned int api_ver)
{
    if (api_ver == 0 || api_ver > MIST_ENGINE_API_MAX_VERSION)
    {
        return MIST_VERSION_NOT_SUPPORTED;
    }
    
    mist_api_version = api_ver;
    return MIST_OK;
}

EMistErrorCode
mist_tg_create(CMistTGroup** ptg,
    const CMistNameValuePair* source, size_t num,
    size_t main_index,
    const char* begin_marker, const char* end_marker,
    size_t* bad_index, char** error_descr)
{
    if (mist_api_version == 0)    
    {
        return MIST_LIBRARY_NOT_INITIALIZED;
    }
    
    assert(ptg != NULL);
    assert(source != NULL);
    assert(begin_marker != NULL);
    assert(begin_marker[0] != '\0');
    assert(end_marker != NULL);
    assert(end_marker[0] != '\0');
    assert(bad_index != NULL);
    
    assert(num > 0);
    assert(main_index < num);
    if (num == 0 || main_index >= num ||
        begin_marker[0] == '\0' || end_marker[0] == '\0')
    {
        return MIST_UNSPECIFIED_ERROR;
    }
    
    *ptg = NULL;
    *bad_index = (size_t)(-1);
    
    CGrowingArray tpl_names;
    CGrowingArray tpl_strings;
    
    if (!grar_create(&tpl_names))
    {
        return MIST_OUT_OF_MEMORY;
    }
    if (!grar_reserve(&tpl_names, num))
    {
        grar_destroy(&tpl_names);
        return MIST_OUT_OF_MEMORY;
    }
    
    if (!grar_create(&tpl_strings))
    {
        grar_destroy(&tpl_names);
        return MIST_OUT_OF_MEMORY;
    }
    if (!grar_reserve(&tpl_strings, num))
    {
        grar_destroy(&tpl_names);
        grar_destroy(&tpl_strings);
        return MIST_OUT_OF_MEMORY;
    }
    
    for (size_t i = 0; i < num; ++i)
    {
        if (!grar_add_element(&tpl_names, (TGAElem)(source[i].name)) ||
            !grar_add_element(&tpl_strings, (TGAElem)(source[i].val)))
        {
            grar_destroy(&tpl_names);
            grar_destroy(&tpl_strings);
            return MIST_OUT_OF_MEMORY;
        }
    }
    
    *ptg = mist_tg_create_impl(source[main_index].name, 
        &tpl_names, &tpl_strings,
        begin_marker, end_marker, 
        bad_index, error_descr);

    grar_destroy(&tpl_names);
    grar_destroy(&tpl_strings);
    
    if (*ptg == NULL)
    {
        return MIST_UNSPECIFIED_ERROR;
        // Not quite informative but if '*error_descr' is not NULL,
        // it may clarify the situation.
        // If it is NULL, it is more likely to be an out-of-memory error.
    }
    return MIST_OK;
}

EMistErrorCode
mist_tg_create_single(CMistTGroup** ptg,
    const char* name, const char* str,
    const char* begin_marker, const char* end_marker,
    char** error_descr)
{
    if (mist_api_version == 0)    
    {
        return MIST_LIBRARY_NOT_INITIALIZED;
    }
    
    assert(ptg != NULL);
    assert(name != NULL);
    assert(name[0] != '\0');
    assert(str != NULL);
    assert(begin_marker != NULL);
    assert(begin_marker[0] != '\0');
    assert(end_marker != NULL);
    assert(end_marker[0] != '\0');
    
    if (name[0] == '\0' ||
        begin_marker[0] == '\0' || end_marker[0] == '\0')
    {
        return MIST_UNSPECIFIED_ERROR;
    }
    
    *ptg = mist_tg_create_single_impl(name, str, begin_marker, end_marker, error_descr);
    if (*ptg == NULL)
    {
        return MIST_UNSPECIFIED_ERROR;
        // Not quite informative but if '*error_descr' is not NULL,
        // it may clarify the situation.
        // If it is NULL, it is more likely to be an out-of-memory error.
    }
    return MIST_OK;
}

EMistErrorCode
mist_tg_destroy(CMistTGroup* mtg)
{
    if (mist_api_version == 0)    
    {
        return MIST_LIBRARY_NOT_INITIALIZED;
    }
    
    assert(mtg != NULL);
    mist_tg_destroy_impl(mtg);
    
    return MIST_OK;
}

EMistErrorCode
mist_tg_add_value(CMistTGroup* mtg, const char* name, const char* val)
{
    if (mist_api_version == 0)    
    {
        return MIST_LIBRARY_NOT_INITIALIZED;
    }
    
    assert(mtg != NULL);
    assert(name != NULL);
    assert(name[0] != '\0');
    assert(val != NULL);
    
    if (name[0] == '\0')
    {
        return MIST_UNSPECIFIED_ERROR;
    }
    
    return mist_tg_add_value_impl(mtg, name, val);
}

EMistErrorCode
mist_tg_set_values(CMistTGroup* mtg, 
    const CMistNameValuePair* attrs, size_t num)
{
    if (mist_api_version == 0)    
    {
        return MIST_LIBRARY_NOT_INITIALIZED;
    }
    
    if (num == 0)
    {
        // Nothing to do
        return MIST_OK;
    }
    assert(mtg != NULL);
    assert(attrs != NULL);
    
    CStringMap* sm = smap_create();
    if (sm == NULL)
    {
        return MIST_OUT_OF_MEMORY;
    }
    
    for (size_t i = 0; i < num; ++i)
    {
        if (!smap_add_element(sm, attrs[i].name, attrs[i].val))
        {
            smap_destroy(sm);
            return MIST_OUT_OF_MEMORY;
        }
    }
    
    EMistErrorCode ec = mist_tg_set_values_impl(mtg, sm);
    smap_destroy(sm);
    
    return ec;
}

EMistErrorCode
mist_tg_clear_values(CMistTGroup* mtg)
{
    if (mist_api_version == 0)    
    {
        return MIST_LIBRARY_NOT_INITIALIZED;
    }
    
    assert(mtg != NULL);
    
    mist_tg_clear_values_impl(mtg);    
    return MIST_OK;
}

EMistErrorCode
mist_tg_evaluate(CMistTGroup* mtg, 
    const char*** presult, size_t* nvals)
{
    if (mist_api_version == 0)    
    {
        return MIST_LIBRARY_NOT_INITIALIZED;
    }
    
    assert(mtg != NULL);
    assert(presult != NULL);
    assert(nvals != NULL);
    
    *presult = NULL;
    
    CGrowingArray* vals = mist_tg_evaluate_impl(mtg);
    if (vals == NULL)
    {
        return MIST_OUT_OF_MEMORY;
    }
    
    *nvals = grar_get_size(vals);
    assert(*nvals > 0); // there must be at least one value
    
    *presult = grar_get_c_array(vals, const char*);
    assert(*presult != NULL);

    return MIST_OK;
}
///////////////////////////////////////////////////////////////////////////
