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

#include "opencv2/core/core.hpp"
#include "opencv2/flann/miniflann.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/photo/photo.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/core/core_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/highgui/highgui_c.h"

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
				decode(messageJSON.Get("picture"));
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
		
		cv::Mat image;
		image = cv::imread("sample.jpg");
		
		if( image.empty() ) {
			log("Sample image was unable to be loaded");
		} else {
			log("sample image successfully loaded");
		}

		 char buff[10000];
		 char *path = getcwd( buff, 10000 );
		
		 std::string cwd( buff );
		
		 log(cwd + std::string("Hi there"));

		//cv::imwrite("432findmetoo.jpg",image);
		
		
		log("OpenCV initialization complete");
	}
	
	/* open (std::string file)
	 * open a file using as a resource using the string passed by the client.
	 */
	void open(std::string file) {
	}
	
	void decode(pp::Var picture) {
		pp::VarArray pic(picture);
		char buff[10000];
		sprintf(buff,"test %d",pic.GetLength());
		log(std::string(buff));
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
