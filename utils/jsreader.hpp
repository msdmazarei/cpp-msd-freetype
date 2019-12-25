#ifndef _MSD_JSREADER_
#define _MSD_JSREADER_
#include <defs/typedefs.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(void *, do_fetch, (DWORD offset,DWORD len), {
  return Asyncify.handleSleep(async function(wakeUp) {
    debugger;
    out("Wasm: waiting for a fetch");
    let k = msd_js_function_read_bytes(offset,len);
    k.then(res =>{
        //res should be a dictionary like: {"data": UInt8Array }
        let l = res['data'].length;
        let newData = new Uint8Array(l + 4);
        newData.set(res['data'],4);
        for(let i=0;i<4;i++){
            let v1 = (1<<(8*i))*0xff;
            newData[i] = (l & v1) >> (8*i)
        }
        let mptr = copyBufferToHeap(newData);
      out("WASM: k resolved.");

        wakeUp(mptr);
    //   wakeUp(res);
    }, rej => {
      out("WASM: k rejected");
      wakeUp(0);
    });
    await k;
    // fetch("a.html").then(response => {
    //   out("got the fetch response");
    //   // (normally you would do something with the fetch here)
    //   wakeUp();
    // });
  });
});
#else

void * js_do_fetch(DWORD offset, DWORD len){
    return 0;
};

#endif

#endif