#include <stdio.h>
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
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
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"


class VitileegoCVEngine : public pp::Instance {
	public:
	explicit VitileegoCVEngine(PP_Instance instance) : pp::Instance(instance){}
	virtual ~VitileegoCVEngine(){}
	
	virtual void HandleMessage(const pp::Var& message) {
		if(!message.is_string())
			return;
		char tmessage[1024];
		sprintf(tmessage,"%s %s",message.AsString().c_str(), "! Hi there :)");
		//fprintf(stdout,"message: %s",tmessage);
		pp::Var reply = pp::Var(tmessage);
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
