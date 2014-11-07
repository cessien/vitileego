#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cmath>
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
	bool image_loaded;
	
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
				image_loaded = true;
			} else if (fn == "adjust") {
				set_threshold(messageJSON.Get("threshold").AsInt());
				set_ratio(messageJSON.Get("ratio").AsInt());
				set_kernel_size(messageJSON.Get("kernel_size").AsInt());
				set_min_slope(messageJSON.Get("min_slope").AsDouble());
				if(messageJSON.HasKey("step1"))
					set_step1(messageJSON.Get("step1").AsBool());
				if(messageJSON.HasKey("step2"))
					set_step2(messageJSON.Get("step2").AsBool());
				if(messageJSON.HasKey("step3"))
					set_step3(messageJSON.Get("step3").AsBool());
				if(messageJSON.HasKey("use_mask"))
					set_use_mask(messageJSON.Get("use_mask").AsBool());
				if(image_loaded)
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
	double min_slope;
	bool step1;
	bool step2;
	bool step3;
	bool use_mask;
	
	void init_defaults(){
		image_loaded = false;
		low_threshold = 30;
		ratio = 3;
		kernel_size = 3;
		min_slope = 1;
		step1 = true;
		step2 = true;
		step3 = true;
		use_mask = false;
	}
	
	void init() {
		init_defaults();
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
	
	void set_min_slope( double param ) {
		min_slope = param;
	}
	
	void set_step1( bool param ) {
		step1 = param;
	}
	
	void set_step2( bool param ) {
		step2 = param;
	}
	
	void set_step3( bool param ) {
		step3 = param;
	}
	
	void set_use_mask( bool param) {
		use_mask = param;
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
		
		reprocess();
	}
	
	void reprocess() {
		cv::RNG rng(12345);
		image = cv::imdecode(cv::Mat(input_data), CV_LOAD_IMAGE_COLOR);
		
		double new_scale = getNewScale(image.size().width, image.size().height);
		log("new scale: " +  std::to_string(new_scale));
		cv::resize(image, image, cv::Size(), new_scale, new_scale, CV_INTER_LINEAR );
		
		
		if(image.empty()){
			log("hmm");
			if(input_data.empty()) {log("with an extra hmmmmmmm");}
		} else {
			char buff[10000];
			sprintf(buff,"[reprocess] size: %i",image.total());
			log(buff);
		}
		
		log("received");
		
		cv::Mat src_gray;
		
		cv::Mat dst, detected_edges;
		
		std::vector<std::vector<cv::Point> > cont;
		
		std::vector<cv::Vec4i> hierarchy;
		
		dst.create(image.size(), image.type());
		
		cv::cvtColor(image, src_gray, CV_BGR2GRAY);
		
		cv::blur(src_gray, detected_edges, cv::Size(2,2));
		
		cv::Canny(detected_edges, detected_edges, low_threshold, low_threshold*ratio, kernel_size);
		
		cv::threshold(detected_edges, detected_edges, 0, 255, 3);
		
		cv::findContours( detected_edges, cont, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
		
		cv::Mat drawing = cv::Mat::zeros( detected_edges.size(), CV_8UC3 );
		
		/* This algorithm eliminates options by the positioning of their lowest point. If its above the halfway point of the screen consider it gone. */
		if (step1) 
			tailAlgorithm1(&cont, image.size().height/2);
		else log("skipping step 1...");
		
		/* This algorithm looks for a tail based on the  threshold slope iterating up from the bottom. */
		if (step2) 
			tailAlgorithm2(&cont, 1, 20);
		else log("skipping step 2...");
		
		removeShortContours(&cont);
		
		/* Run it again with a higher resolution, and more tolerance for slope */
		if (step3) 
			tailAlgorithm2(&cont, min_slope, 5);
		else log("skipping step 3...");
		
		for( int i = 0; i< cont.size(); i++ ) {
			cv::Scalar color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
			//if (use_mask) 
				cv::drawContours( image, cont, i, color, CV_FILLED, 8, hierarchy, 0, cv::Point() );
			if(cont[i].size() != 0) {
				cv::circle( image, cont[i][findLowPoint(cont[i])], 5, cv::Scalar( 0, 0, 255 ), 1, 8 );
				if (use_mask) drawPoints( &image, cont[i] );
			}
		}
		
		dst = cv::Scalar::all(0);
		
		if (use_mask) 
			image.copyTo(dst, detected_edges);
		else
			image.copyTo(dst);
		//image.copyTo(dst, drawing);
		
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
		//context.Flush(callback_factory.NewCallback(&VitileegoCVEngine::render_loop));
	}
	
	private:
	void log(std::string message) {
		pp::VarDictionary reply;
		reply.Set("type","log");
		reply.Set("message",message);
		PostMessage(reply);
	}
	
	int findLowPoint(std::vector<cv::Point> contour) {
		cv::Point lowest_point = contour[0];
		int index = 0;
		for(int i =0; i < contour.size(); i++) {
			//log(std::string("point: ") + std::to_string(contour[i].x) + std::string(", ") + std::to_string(contour[i].y));
			if(contour[i].y > lowest_point.y) {
				lowest_point = contour[i];
				index = i;
			}
		}
		//log(std::string("point: ") + std::to_string(lowest_point.x) + std::string(", ") + std::to_string(lowest_point.y));
		return index;
	}
	
	void tailAlgorithm1(std::vector<std::vector<cv::Point> > * contour, int threshold_height) {
		for ( std::vector<std::vector<cv::Point> >::iterator it = contour->begin(); it != contour->end(); ++it) {
			int i = it - contour->begin();
			// If the specific contour's low point is above the halfway
			std::vector<cv::Point> temp = (*contour)[i];
			if( (*contour)[i][findLowPoint(temp)].y < threshold_height ) {
				(*contour)[i].clear();
			}
		}
	}
	
	void tailAlgorithm2(std::vector<std::vector<cv::Point> > * contour, double min_slope, int resolution){
		log("min slope " + std::to_string(min_slope));
		for (int j = 0; j < contour->size(); j++) {
			log("----------------------------------------");
			int low_point_index = findLowPoint((*contour)[j]);
			cv::Point old_point = (*contour)[j][low_point_index];
			
			/****** From the center point work right... *************/
			for (std::vector<cv::Point>::iterator it = (*contour)[j].begin() + low_point_index; it != (*contour)[j].end(); ++it) {
				//only count points that are a distance of 10 pixels vertically
				//log("curr y: " + std::to_string(it->y) + "old y: " + std::to_string(old_point.y));
				if(old_point.y - resolution < it->y) continue;
				//Get the next resolution point
				cv::Point current_point(it->x,it->y);
				//Calculate the slope
				double slope = (double)(current_point.y - old_point.y)/(double)(current_point.x - old_point.x);
				slope = (double)std::abs(slope);
				
				old_point.x = current_point.x;
				old_point.y = current_point.y;
				if ((double)slope < (double)min_slope) {
					(*contour)[j] = std::vector<cv::Point>((*contour)[j].begin(),it);
					break;
				}
			}
			
			old_point = (*contour)[j][low_point_index];
			/***** Annnnnnnd reverse *********/
			for (std::vector<cv::Point>::iterator it = (*contour)[j].begin() + low_point_index; it != (*contour)[j].begin(); --it) {
				//only count points that are a distance of 10 pixels vertically
				//log("curr y: " + std::to_string(it->y) + "old y: " + std::to_string(old_point.y));
				if(old_point.y - resolution < it->y) continue;
				//Get the next resolution point
				cv::Point current_point(it->x,it->y);
				//Calculate the slope
				double slope = (double)(current_point.y - old_point.y)/(double)(current_point.x - old_point.x);
				//log(std::to_string(slope));
				slope = (double)std::abs(slope);
				
				old_point.x = current_point.x;
				old_point.y = current_point.y;
				if ((double)slope < (double)min_slope) {
					(*contour)[j] = std::vector<cv::Point>(it,(*contour)[j].end());
					break;
				}
			}
		}
	}
	
	void removeShortContours(std::vector<std::vector<cv::Point> > * contour){
		for (int j = 0; j < contour->size(); j++) {
			double distance = 0.0;
			cv::Point old_point = (*contour)[j][0];
			
			for (std::vector<cv::Point>::iterator it = (*contour)[j].begin(); it != (*contour)[j].end(); ++it) {
				cv::Point current_point(it->x,it->y);
				//Calculate the distance
				double curr_distance = std::sqrt((double)(std::pow(current_point.x - old_point.x,2) + std::pow(current_point.y - old_point.y,2)));
				curr_distance = (double)std::abs(curr_distance);
				
				distance += curr_distance;
				
				old_point.x = current_point.x;
				old_point.y = current_point.y;
			}
			
			if(distance < 100) {
				(*contour)[j].clear();
			}
		}
	}
	
	void drawPoints(cv::Mat * img, std::vector<cv::Point> contour) {
		for(int k = 0; k < contour.size(); k++) {
			cv::circle( *img, contour[k], 2, cv::Scalar( 255, 0, 0 ), 1, 8 );
		}
	}
	
	/* Takes an image size and scales it to best fit the 800*600 screen */
	double getNewScale(int width, int height) {
		//Scale the largest dimension of an image to the smallest of 800*600
		return (width > height) ? (double)800/(double)width : (double)600/(double)height;
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
