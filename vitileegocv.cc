#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ppapi/cpp/graphics_2d.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/var_dictionary.h"
#include "ppapi/c/ppb_image_data.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/utility/completion_callback_factory.h"

#include "opencv2/opencv.hpp"

class VitileegoCVEngine : public pp::Instance {
	pp::Graphics2D context;
	pp::Size size;
	pp::CompletionCallbackFactory<VitileegoCVEngine> callback_factory;
	pp::ImageData image_data;
	
	public:
	explicit VitileegoCVEngine(PP_Instance instance) : pp::Instance(instance), callback_factory(this){}
	virtual ~VitileegoCVEngine(){}
	
	/* void HandleMessage(pp:var message)
	 * This is the main interface for working client side messages on the presentation layer.
	 * The javascript can communicate either a JSON object, or string variable, image data object. JSON object should be 
	 * the only type fully expected though. The JSON object is required to have a type attribute that specifies the requested 
	 * action of the engine. The fn should be a function. based on the function other parameters are assumed to be passed, 
	 * which will be pulled here.
	 */
	virtual void HandleMessage(const pp::Var& message) {
		if(message.is_dictionary()) {
			pp::VarDictionary messageJSON(message);
			std::string fn = messageJSON.Get("fn").AsString(); 
			if (fn == "init") { //initialize (or re-initialize) opencv
				init();
			} else if (fn == "open") { //open a specific file
				open(messageJSON.Get("file").AsString());
			} else if (fn == "picture") { //decode the picture
				decode(messageJSON.Get("picture"),messageJSON.Get("size").AsInt());
			}
			

		} else if (message.is_string()) { //Simple dummy test to respond
			char tmessage[1024];
			sprintf(tmessage,"%s %s",message.AsString().c_str(), "! Hi there :)");
			pp::VarDictionary reply;
			reply.Set("message",tmessage);
		
			PostMessage(reply);
		} else { //Assume possible image_data
			
		}	
	}
	
	void init() {
		size = pp::Size(800,600);
		context = pp::Graphics2D(this, size,true);
		
		if (!BindGraphics(context)) log("context wasn't binded");
		
		PP_ImageDataFormat format = pp::ImageData::GetNativeImageDataFormat();
    image_data = pp::ImageData(this, format, size, true);
		
		uint32_t* data = static_cast<uint32_t*>(image_data.data());
		
		uint8_t test = 128;
		uint8_t opaque = 0;
		
		for (int i = 0; i < 800*600; i++) {
			data[i] = 0xffff0000;
			//data[i] = (test << 24) | (test << 16) | (test << 8) | opaque;
		}
		
		context.ReplaceContents(&image_data);
		
		//render_loop(0);
		context.Flush(callback_factory.NewCallback(&VitileegoCVEngine::render_loop));
		
		log("OpenCV initialization complete");
	}
	
	/* open (std::string file)
	 * open a file using as a resource using the string passed by the client.
	 */
	void open(std::string file) {
	}
		
	void decode(const pp::Var & pictureData,int size) {
		if(pictureData.is_object()){
			log("Im an array buffer!");
		} else {
			log("or not..");
		}
		
		pp::VarArrayBuffer picArray(pictureData);			
		std::vector<uint8_t> test(0);
		
		char* data = static_cast<char*>(picArray.Map());
		uint32_t byte_length = picArray.ByteLength();

		//for (uint32_t i = 0; i < byte_length; i++) {
			//test.push_back(data[1]);
		//}
		
		char buff[10000];
		sprintf(buff,"size?: %i %i",byte_length, size);
		log(std::string(buff));
		/*
		if(test.empty()){
			log("hmm");
		} else {
			char buff[10000];
			sprintf(buff,"fact: %x %x %x %x %x %x %x %x %x %x\n%x %x %x %x %x %x %x %x %x %x\n%x %x %x %x %x %x %x %x %x %x\n%x %x",test[0], test[1], test[2], test[3], test[4], test[5], test[6], test[7], test[8], test[9], test[10], test[11], test[12], test[13], test[14], test[15], test[16], test[17], test[18], test[19], test[20], test[21], test[22], test[23], test[24], test[25], test[26], test[27], test[28], test[29], test[30], test[31]);
			log(std::string(buff));
		} */
		
		/*									 
		if(test.empty()){
			log("hmm");
		} else {
			char buff[10000];
			sprintf(buff,"fact: %x %x %x %x %x %x %x %x %x %x\n%x %x %x %x %x %x %x %x %x %x\n%x %x %x %x %x %x %x %x %x %x\n%x %x",test[0], test[1], test[2], test[3], test[4], test[5], test[6], test[7], test[8], test[9], test[10], test[11], test[12], test[13], test[14], test[15], test[16], test[17], test[18], test[19], test[20], test[21], test[22], test[23], test[24], test[25], test[26], test[27], test[28], test[29], test[30], test[31]);
			log(std::string(buff));
		}
	
			cv::Mat image = cv::imdecode(cv::Mat(test), CV_LOAD_IMAGE_COLOR);


			if( image.empty() ) {
				log("Sample image was unable to be loaded");
			} else {
				log("sample image successfully loaded");
			}
*/
		
	}
	
	void render_loop(int32_t){
		log("called");
		//context.Flush(callback_factory.NewCallback(&VitileegoCVEngine::render_loop));
	}
	
	private:
	void log(std::string message) {
		pp::VarDictionary reply;
		reply.Set("type","log");
		reply.Set("message",message);
		PostMessage(reply);
	}
};

class VitileegoCVEngineModule: public pp::Module {
	public:
	virtual ~VitileegoCVEngineModule() {}

	virtual pp::Instance* CreateInstance(PP_Instance instance) {
		
		return new VitileegoCVEngine(instance);
	};
};

namespace pp {
	Module* CreateModule() {
		return new VitileegoCVEngineModule();
	}
}
