#pragma once
#include "Arduino.h"

#include "Array.h"
#include <FS.h>

#if !defined(ESP8266)
#include <SPIFFS.h>
#endif

template<class T>
class CHistory :public CSimpleArray <T>{
public:
	typedef void(*WriteElementFN)(Stream* ps,T elem);
	typedef String(*GetStringElement)( T elem);
  typedef T(*ExtractFromJsonString)(String json);
 
	void WriteToStream(Stream* ps) {
		for (int i = 0; i < this->GetSize(); i++) {
			pfn_write_element(ps,this->GetAt(i));
		}
	}
	bool WriteToFile(String fileName) {
    if(!pfn_get_string_element)
      return false;
    SPIFFS.remove(fileName);
    
		File file = SPIFFS.open(fileName, "w+");
		if (!file){
      Serial.println("Failed to open file");
			return false;
		}
		file.print("[\n");
		for (int i = 0; i < this->GetSize(); i++) {
   
			String s = pfn_get_string_element(this->GetAt(i));
			file.print(s);
			if (i == (this->GetSize() - 1)) {
				file.print("\n");
			}
			else {
				file.print(",\n");
			}

		}
		file.print("]");
		file.close();
		return true;
	};
  bool LoadFromFile(String fileName){
    File file = SPIFFS.open(fileName, "r");
    if (!file){
      Serial.println("Failed to open file");
      return false;
    }
    int line=0;
    String buffer;
    while (file.available()) {
      buffer = file.readStringUntil('\n');
      //Serial.println(buffer); //Printing for debugging purpose         
      line++;
      if(line==1){
        continue;
      }
      if(buffer=="]"){
        break;
      }
      if(buffer.endsWith(",")){
        buffer.remove(buffer.length()-1,1);
      }
      if(pfn_extract_from_jsonstring){
        T elem=pfn_extract_from_jsonstring(buffer);
        this->Add(elem);
      }
    }
  };
	void SetWriteFn(WriteElementFN fn) { pfn_write_element = fn; };
	void SetGetStringFn(GetStringElement fn) { pfn_get_string_element = fn; };
  void SetExtractFn(ExtractFromJsonString fn) { pfn_extract_from_jsonstring = fn; };
private:
	WriteElementFN pfn_write_element;
	GetStringElement pfn_get_string_element;
  ExtractFromJsonString pfn_extract_from_jsonstring;
};
