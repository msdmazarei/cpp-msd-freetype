#ifndef _MSD_JSREADER_
#define _MSD_JSREADER_
#include <defs/typedefs.hpp>

#ifdef __EMSCRIPTEN_minor__
#include <emscripten.h>

EM_JS(DWORD , js_do_fetch, (DWORD id,DWORD offset,DWORD len,void* memptr), {
  return Asyncify.handleSleep(function(wakeUp) {
    debugger;
    out("Wasm: waiting for a fetch");
    out("id:",id);
    let k = msd_js_function_read_bytes(offset,len);
    k.then(res =>{
      debugger;
      wakeUp(0);
        //res should be a dictionary like: {"data": UInt8Array }
        let l = res['data'].length;
        for(let i=0;i<l;i++) Moudle.HEAPU8[memptr+i]=res["data"][i];
        
        // let newData = new Uint8Array(l + 4);
        // newData.set(res['data'],4);
        // for(let i=0;i<4;i++){
        //     let v1 = (1<<(8*i))*0xff;
        //     newData[i] = (l & v1) >> (8*i)
        // }
        // let mptr = copyBufferToHeap(newData);
      out("WASM: k resolved.");

        wakeUp(l);
    //   wakeUp(res);
    }, rej => {
      out("WASM: k rejected");
      wakeUp(0);
    });
    // await k;
    // fetch("a.html").then(response => {
    //   out("got the fetch response");
    //   // (normally you would do something with the fetch here)
    //   wakeUp();
    // });
  });
});
#else

DWORD  js_do_fetch(DWORD id,DWORD offset, DWORD len, void* memptr){
    return 0;
};

#endif

#endif