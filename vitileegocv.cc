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
	PP_ImageDataFormat format;
	uint32_t* data;
	std::vector<uint8_t> input_data;
	cv::Mat image;
	
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
				decode(messageJSON.Get("picture").AsString(),messageJSON.Get("size").AsInt());
			} else if (fn == "adjust") {
				set_threshold(messageJSON.Get("threshold").AsInt());
				set_ratio(messageJSON.Get("ratio").AsInt());
				set_kernel_size(messageJSON.Get("kernel_size").AsInt());
				reprocess();
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
	
	int low_threshold;
	int ratio;
	int kernel_size;
	
	void init() {
		low_threshold = 0;
		ratio = 250;
		kernel_size = 5;
		
		size = pp::Size(800,600);
		context = pp::Graphics2D(this, size,true);
		
		if (!BindGraphics(context)) log("context wasn't binded");
		
		format = pp::ImageData::GetNativeImageDataFormat();
    image_data = pp::ImageData(this, format, size, true);
		
		data = static_cast<uint32_t*>(image_data.data());
		
		uint8_t test = 128;
		uint8_t opaque = 0;
		
		for (int i = 0; i < 800*600; i++) {
			// bitwise this data is separated into 4 bytes alpha|red|green|blue
			data[i] = (0xff << 24) | (0x00 << 16) | (0xff << 8) | 0x00;
		}
		
		printf("testing!!\n");
		
		context.ReplaceContents(&image_data);
		
		//render_loop(0);
		context.Flush(callback_factory.NewCallback(&VitileegoCVEngine::render_loop));
		
		log("OpenCV initialization complete");
	}
	
	void set_threshold( int param ) {
		low_threshold = param;
		char buff[10000];
		sprintf(buff,"testing testing 123: %i", low_threshold);
		log(buff);
	}
	
	void set_ratio( int param ) {
		ratio = param;
	}
	
	void set_kernel_size( int param ) {
		kernel_size = param;
	}
	
	/* open (std::string file)
	 * open a file using as a resource using the string passed by the client.
	 */
	void open(std::string file) {
	}
		
	void decode(std::string pictureData,int size) {
		std::string decoded_data = base64_decode(pictureData);
		const uint8_t * temp  = (uint8_t *)decoded_data.c_str();
		input_data = std::vector<uint8_t>(decoded_data.size());
		

		for (uint32_t i = 0; i < decoded_data.size(); i++) {
			input_data[i]=temp[i];
		}
		
		
		image = cv::imdecode(cv::Mat(input_data), CV_LOAD_IMAGE_COLOR);
		
		if(image.empty()){
			log("hmm");
			if(input_data.empty()) {log("with an extra hmmmmmmm");}
		} else {
			char buff[10000];
			//cv::Size s = 
			sprintf(buff,"size: %i",image.total());
			log(buff);
		}
		
		cv::Mat src_gray;
		
		cv::Mat dst, detected_edges;
		
		dst.create(image.size(), image.type());
		
		cv::cvtColor(image, src_gray, CV_BGR2GRAY);
		
		cv::blur(src_gray, detected_edges, cv::Size(3,3));
		
		cv::Canny(detected_edges, detected_edges, low_threshold, low_threshold*ratio, kernel_size);
		
		dst = cv::Scalar::all(0);
		
		image.copyTo(dst, detected_edges);
		
		PP_ImageDataFormat format = pp::ImageData::GetNativeImageDataFormat();
    image_data = pp::ImageData(this, format, pp::Size(800,600), true);
		
		data = static_cast<uint32_t*>(image_data.data());
		
		for (int x = 0; x < dst.rows; x++) {
			unsigned char * row = dst.ptr(x);
			for (int y = 0; y < dst.cols*3; y+=3) {
				uint8_t b = *(row+(y+0));
				uint8_t g = *(row+(y+1));
				uint8_t r = *(row+(y+2));
				data[x*800 + y/3] = (0xff << 24) | (r << 16) | (g << 8) | b;
			}
		}
		
		context.ReplaceContents(&image_data);
		context.Flush(callback_factory.NewCallback(&VitileegoCVEngine::render_loop));
	}
	
	void reprocess() {
		image = cv::imdecode(cv::Mat(input_data), CV_LOAD_IMAGE_COLOR);
		
		if(image.empty()){
			log("hmm");
			if(input_data.empty()) {log("with an extra hmmmmmmm");}
		} else {
			char buff[10000];
			//cv::Size s = 
			sprintf(buff,"[reprocess] size: %i",image.total());
			log(buff);
		}
		
		log("received");
		
		cv::Mat src_gray;
		
		cv::Mat dst, detected_edges;
		
		dst.create(image.size(), image.type());
		
		cv::cvtColor(image, src_gray, CV_BGR2GRAY);
		
		cv::blur(src_gray, detected_edges, cv::Size(3,3));
		
		cv::Canny(detected_edges, detected_edges, low_threshold, low_threshold*ratio, kernel_size);
		
		dst = cv::Scalar::all(0);
		
		image.copyTo(dst, detected_edges);
		
		PP_ImageDataFormat format = pp::ImageData::GetNativeImageDataFormat();
    image_data = pp::ImageData(this, format, pp::Size(800,600), true);
		
		data = static_cast<uint32_t*>(image_data.data());
		
		for (int x = 0; x < dst.rows; x++) {
			unsigned char * row = dst.ptr(x);
			for (int y = 0; y < dst.cols*3; y+=3) {
				uint8_t b = *(row+(y+0));
				uint8_t g = *(row+(y+1));
				uint8_t r = *(row+(y+2));
				data[x*800 + y/3] = (0xff << 24) | (r << 16) | (g << 8) | b;
			}
		}
		
		context.ReplaceContents(&image_data);
		context.Flush(callback_factory.NewCallback(&VitileegoCVEngine::render_loop));
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
	
	static inline bool is_base64(unsigned char c) {
		return (isalnum(c) || (c == '+') || (c == '/'));
	}
	
	std::string base64_decode(std::string const& encoded_string) {
		const std::string base64_chars = 
							 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
							 "abcdefghijklmnopqrstuvwxyz"
							 "0123456789+/";

		int in_len = encoded_string.size();
		int i = 0;
		int j = 0;
		int in_ = 0;
		unsigned char char_array_4[4], char_array_3[3];
		std::string ret;

		while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
			char_array_4[i++] = encoded_string[in_]; in_++;
			if (i ==4) {
				for (i = 0; i <4; i++)
					char_array_4[i] = base64_chars.find(char_array_4[i]);

				char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
				char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

				for (i = 0; (i < 3); i++)
					ret += char_array_3[i];
				i = 0;
			}
		}

		if (i) {
			for (j = i; j <4; j++)
				char_array_4[j] = 0;

			for (j = 0; j <4; j++)
				char_array_4[j] = base64_chars.find(char_array_4[j]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
		}

		return ret;
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
