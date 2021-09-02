// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME ELossDict
#define R__NO_DEPRECATION

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "RConfig.h"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Header files passed as explicit arguments
#include "ELossGUI.h"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static TClass *MyMainFrame_Dictionary();
   static void MyMainFrame_TClassManip(TClass*);
   static void delete_MyMainFrame(void *p);
   static void deleteArray_MyMainFrame(void *p);
   static void destruct_MyMainFrame(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::MyMainFrame*)
   {
      ::MyMainFrame *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::MyMainFrame));
      static ::ROOT::TGenericClassInfo 
         instance("MyMainFrame", "ELossGUI.h", 40,
                  typeid(::MyMainFrame), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &MyMainFrame_Dictionary, isa_proxy, 0,
                  sizeof(::MyMainFrame) );
      instance.SetDelete(&delete_MyMainFrame);
      instance.SetDeleteArray(&deleteArray_MyMainFrame);
      instance.SetDestructor(&destruct_MyMainFrame);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::MyMainFrame*)
   {
      return GenerateInitInstanceLocal((::MyMainFrame*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::MyMainFrame*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *MyMainFrame_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::MyMainFrame*)0x0)->GetClass();
      MyMainFrame_TClassManip(theClass);
   return theClass;
   }

   static void MyMainFrame_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrapper around operator delete
   static void delete_MyMainFrame(void *p) {
      delete ((::MyMainFrame*)p);
   }
   static void deleteArray_MyMainFrame(void *p) {
      delete [] ((::MyMainFrame*)p);
   }
   static void destruct_MyMainFrame(void *p) {
      typedef ::MyMainFrame current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::MyMainFrame

namespace {
  void TriggerDictionaryInitialization_ELossDict_Impl() {
    static const char* headers[] = {
"ELossGUI.h",
0
    };
    static const char* includePaths[] = {
"/home/software/root/include/",
"/home/valdre/Documenti/exp-dev/elarc/modules/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "ELossDict dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
class __attribute__((annotate("$clingAutoload$ELossGUI.h")))  MyMainFrame;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "ELossDict dictionary payload"


#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "ELossGUI.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"MyMainFrame", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("ELossDict",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_ELossDict_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_ELossDict_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_ELossDict() {
  TriggerDictionaryInitialization_ELossDict_Impl();
}
