#ifndef _MSD_JSREADER_
#define _MSD_JSREADER_
#include <defs/typedefs.hpp>

#ifdef __EMSCRIPTEN_minor__
#include <emscripten.h>

EM_JS(DWORD , js_do_fetch, (DWORD id,DWORD offset,DWORD len,void* memptr), {
  return Asyncify.handleSleep( function(wakeUp) {
    out("js_do_fetch: waiting for a fetch (id,offset,len,memptr)",id,offset,len,memptr);
    let k = msd_js_function_read_bytes(offset,len);
    k.then(res =>{
        //res should be a dictionary like: {"data": UInt8Array }
        let l = res['data'].length;
        for(let i=0;i<l;i++) HEAPU8[memptr+i]=res["data"][i];
        out("js_do_fetch: k resolved.");
        wakeUp(l);
    }, rej => {
      out("js_do_fetch: k rejected");
      wakeUp(0);
    });
  });
});
#else

DWORD  js_do_fetch(DWORD id,DWORD offset, DWORD len, void* memptr){
    return 0;
};

#endif

#endif