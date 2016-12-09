/*
* Copyright (C) 2010 The Android Open Source Project
*
* Apache License Version 2.0 (�u�{���C�Z���X�v) �Ɋ�Â��ă��C�Z���X����܂��B;
* �{���C�Z���X�ɏ������Ȃ��ꍇ�͂��̃t�@�C�����g�p�ł��܂���B
* �{���C�Z���X�̃R�s�[�́A�ȉ��̏ꏊ�������ł��܂��B
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* �K�p�����@�߂܂��͏��ʂł̓��ӂɂ���Ė������Ȃ�����A�{���C�Z���X�Ɋ�Â��ĔЕz�����\�t�g�E�F�A�́A
* �����َ����킸�A�����Ȃ�ۏ؂��������Ȃ��Ɍ���̂܂�
* �Еz����܂��B
* �{���C�Z���X�ł̌����Ɛ������K�肵���������ẮA
* �{���C�Z���X���Q�Ƃ��Ă��������B
*
*/
#include "FFT.hpp"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))

/**
* �ۑ���Ԃ̃f�[�^�ł��B
*/
struct Point3D {
	GLfloat x;
	GLfloat y;
	GLfloat z;
	void Init() {
		this->x = 0.0f;
		this->y = 0.0f;
		this->z = 0.0f;
	}
};

struct WaveData {
	std::vector< std::vector<Point3D> > DrawPlot;
	Point3D** DrawPlots;
	double* RawData;
	double* FFTData;
	double* AnalyzedFFTData;
	int64_t* TimeStmp;
};

class AnalyzeFFT {
	WaveData Wav;
	FftBox* FFTBox;
	int64_t InputDataPos;
	int64_t InputDataNum;
	std::fstream* Output;
	std::fstream* OutputFFT;
	inline int64_t GetSampNum() {
		return InputDataPos / Range;
	}
	inline void InitDataPos() {
		InputDataPos = 0;
	}
	inline void OutputCSVFFT() {
		(*OutputFFT) << "BoxFFT" << ",";
		(*OutputFFT) << GetSampNum() << ",";
		for (auto i = 0; i < Range; i++) {
			(*OutputFFT) << std::abs((*FFTBox)[i])*2/1024 << ",";
		}
		(*OutputFFT) << std::endl;
	}
	inline void OutputCSVRawData() {
		for (int64_t Loop = 0; Loop < Range; Loop++) {
			auto DataPos = (Loop + GetDataPos()) % Range;
			(*Output) << Loop << "," << Wav.RawData[DataPos] << "," << Wav.TimeStmp[DataPos] << std::endl;
		}
	}
	inline int64_t GetDataPos() {
		return InputDataPos % Range;
	}
	inline void NextDataPos() {
		InputDataPos++;
		InputDataNum++;
	}
	inline bool YesSampleEnd() {
		if (InputDataNum > Range) {
			InputDataNum = 0;
			return 1; }
		else { 
			return 0;
		}
	}
	inline void CalcFFTddst() {
		FFTBox->fft();
		for (int64_t i = 0; i < Range; i++) {
			Wav.FFTData[i] = std::abs((*FFTBox)[i])*2/Range;
			Wav.FFTData[i] *= 2 / Range;
		}
	}
	inline void ResizePlot(int DrawAxis,double LocalMax) {
		for (int64_t loop = 0; loop < Range; loop++) {
			Wav.DrawPlots[DrawAxis][loop].y /= 4 * LocalMax;
			Wav.DrawPlots[DrawAxis][loop].y += (DrawAxis - 1) * 1 / 2;
		}
	}
	template<class Data>inline void MakePlotDataFFT(Data* OutputData, int DrawAxis) {
		double LocalMax = 0.0f;
		for (int64_t loop = 0; loop < Range; loop++) {
			Wav.DrawPlots[DrawAxis][loop].x = (GLfloat)(loop - Range / 2) / (Range / 2);
			Wav.DrawPlots[DrawAxis][loop].y = (GLfloat)OutputData[loop] ;
			Wav.DrawPlots[DrawAxis][loop].z = (GLfloat)0.0f;
			if (std::abs(OutputData[loop]) > LocalMax) {
				LocalMax = std::abs(OutputData[loop]);
			}
		}
		ResizePlot(DrawAxis, LocalMax);
	}
	template<class Data>inline void MakePlotData(Data* OutputData, int DrawAxis) {
		double LocalMax = 0.0;
		int64_t DataPos = GetDataPos();
		for (int64_t loop = 0; loop < Range; loop++) {
			Wav.DrawPlots[DrawAxis][loop].x = (GLfloat)(loop - Range / 2) / (Range / 2);
			Wav.DrawPlots[DrawAxis][loop].y = (GLfloat)OutputData[DataPos++ % Range];
			Wav.DrawPlots[DrawAxis][loop].z = (GLfloat)0.0f;
			if (std::abs(OutputData[loop]) > LocalMax) {
				LocalMax = std::abs(OutputData[loop]);
			}
		}
		ResizePlot(DrawAxis, LocalMax);
	}
	inline void OutputCSVAll() {
		OutputCSVRawData();
		OutputCSVFFT();
	}
public:
	inline void InitFFT(const char* DataName, const char* FFTName) {
		InputDataPos = 0;
		InputDataNum = 0;
		Point3D InitPoint;
		InitPoint.x = 0;
		InitPoint.y = 0;
		InitPoint.z = 0;
		std::vector<Point3D> Tmp;
		Wav.DrawPlots = new Point3D*[3];
		for (int64_t i = 0; i < 3; i++) {
			if (Wav.DrawPlots != NULL) {
				Point3D Init;
				Init.Init();
				Wav.DrawPlots[i] = new Point3D[Range]{Init};
			}			
		}
		Tmp.resize(Range,InitPoint);
		Wav.DrawPlot.resize(3, Tmp);
		Wav.FFTData = new double[Range] {0.0};
		Wav.RawData = new double[Range] {0.0};
		Wav.AnalyzedFFTData = new double[Range] {0.0};
		Wav.TimeStmp = new int64_t[Range];
		FFTBox = new FftBox(Range);
		Output = new std::fstream(DataName, std::fstream::out);
		OutputFFT = new std::fstream(FFTName, std::fstream::out);
		(*OutputFFT) <<  "FFTDATA,SampleNum,";
		for (int64_t i = 0; i < Range; i++) {
			(*OutputFFT) << i * 200.0 / Range << ",";
		}
		(*OutputFFT) << std::endl;
	}
	inline ~AnalyzeFFT() {
		delete[] Output;
		delete[] OutputFFT;
		delete[] Wav.FFTData;
		delete[] Wav.RawData;
		delete[] Wav.TimeStmp;
		delete[] Wav.AnalyzedFFTData;
	}
	inline void GenerateFreq(const ASensorEvent* Event) {
		int64_t LocalDataPos = GetDataPos();
		Wav.TimeStmp[LocalDataPos] = Event->timestamp;
		Wav.RawData[LocalDataPos] = std::sin(2*3.1415*12* LocalDataPos);
		Wav.FFTData[LocalDataPos] = std::sin(2 * 3.1415*12 *LocalDataPos);
		NextDataPos();
	}
	inline void SetAxel(const ASensorEvent* Event) {
		int64_t LocalDataPos = GetDataPos();
		if (Event->timestamp != 0) {
			std::complex<double> Comp(0.0, (double)Event->acceleration.z - (double)9.77017211914625278);
			Wav.TimeStmp[LocalDataPos] = Event->timestamp;
			(*FFTBox)[LocalDataPos] = Comp;
			Wav.RawData[LocalDataPos] = (double)Event->acceleration.z - (double)9.77017211914625278;
			NextDataPos();
		}
	}
	inline void SetVel(const ASensorEvent* Event) {
		int64_t LocalDataPos = GetDataPos();
		Wav.TimeStmp[LocalDataPos] = Event->timestamp;
		Wav.RawData[LocalDataPos] = (double)Event->acceleration.z - (double)9.77017211914625278;
		Wav.FFTData[LocalDataPos] = (double)Event->acceleration.z - (double)9.77017211914625278;
		NextDataPos();
	}
	inline void OutputDisp(EGLDisplay* display,EGLSurface* surface) {
		const GLfloat XLay[] = {//���_
			-1.0f,0.0f,0.0f,
			 1.0f,0.0f,0.0f, 
			-1.0f,-0.5f,0.0f,
			 1.0f,-0.5f,0.0f, 
			-1.0f,0.5f,0.0f,
			 1.0f,0.5f,0.0f 
		};
		const GLfloat YLay[] = {//���_
			-0.9f,1.0f,0.0f,
			-0.9f,-1.0f,0.0f };
		glLineWidth(4.0f);
		glClearColor(((float)1.0), 1.0, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glColor4f(1, 0, 0, 0.5f);
		//���_�z��̎w��
		glVertexPointer(3, GL_FLOAT, 0, XLay);
		glEnableClientState(GL_VERTEX_ARRAY);
		//�v���~�e�B�u�̕`��
		glDrawArrays(GL_LINES, 0, 6);
		glDisableClientState(GL_VERTEX_ARRAY);

		glColor4f(0, 0, 1, 0.5f);
		glVertexPointer(3, GL_FLOAT, 0, YLay);
		glEnableClientState(GL_VERTEX_ARRAY);
		//�v���~�e�B�u�̕`��
		glDrawArrays(GL_LINES, 0, 2);
		glDisableClientState(GL_VERTEX_ARRAY);


		glLineWidth(3.0f);
		glColor4f(1.0, 0.0f, 0, 0.0f);
//		glVertexPointer(3, GL_FLOAT, 0, Wav.DrawPlot[0].data());
		glVertexPointer(3, GL_FLOAT, 0, Wav.DrawPlots[0]);

		//���`��
		glEnableClientState(GL_VERTEX_ARRAY);
		glDrawArrays(GL_LINE_STRIP, 0, Range);
		glDisableClientState(GL_VERTEX_ARRAY);

		glColor4f(0.0f, 1.0f, 0, 0.0f);
//		glVertexPointer(3, GL_FLOAT, 0, Wav.DrawPlot[1].data());
		glVertexPointer(3, GL_FLOAT, 0, Wav.DrawPlots[1]);
		glEnableClientState(GL_VERTEX_ARRAY);
		glDrawArrays(GL_LINE_STRIP, 0, Range);
		glDisableClientState(GL_VERTEX_ARRAY);

		glColor4f(0.0f, 0.0f, 1.0, 0.0f);
		glVertexPointer(3, GL_FLOAT, 0, Wav.DrawPlots[2]);
		glEnableClientState(GL_VERTEX_ARRAY);
		glDrawArrays(GL_LINE_STRIP, 0, Range);
		glDisableClientState(GL_VERTEX_ARRAY);
		// �o�b�t�@�̓���ւ�
		eglSwapBuffers((*display),(*surface));

	}
	inline void TryProcess(){
		MakePlotData(Wav.RawData, 0);
		if (YesSampleEnd()) {
			CalcFFTddst();
			MakePlotDataFFT((*FFTBox).GetSpectol(), 1);
			OutputCSVAll ();
		}

	}
};

struct saved_state {
	float angle;
	int32_t x;
	int32_t y;
	AnalyzeFFT FFTUnit;
	AnalyzeFFT FFTUnitTest;
};
/**
* �A�v���̕ۑ���Ԃł��B
*/
struct engine {
	struct android_app* app;

	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	ASensorEventQueue* sensorEventQueue;
	
	int animating;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	int32_t width;
	int32_t height;
	struct saved_state state;
};

/**
* ���݂̕\���� EGL �R���e�L�X�g�����������܂��B
*/
static int engine_init_display(struct engine* engine) {
	// OpenGL ES �� EGL �̏�����

	/*
	* �ړI�̍\���̑����������Ŏw�肵�܂��B
	* �ȉ��ŁA�I���X�N���[�� �E�B���h�E��
	* �݊����̂���A�e�F�Œ� 8 �r�b�g�̃R���|�[�l���g�� EGLConfig ��I�����܂�
	*/
	const EGLint attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};
	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	/* �����ŁA�A�v���P�[�V�����͖ړI�̍\����I�����܂��B���̃T���v���ł́A
	* ���o�����ƈ�v����ŏ��� EGLConfig ��
	* �I������P���ȑI���v���Z�X������܂� */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID �́AANativeWindow_setBuffersGeometry() ��
	* ����Ď󂯎���邱�Ƃ��ۏ؂���Ă��� EGLConfig �̑����ł��B
	* EGLConfig ��I�������炷���ɁAANativeWindow �o�b�t�@�[����v�����邽�߂�
	* EGL_NATIVE_VISUAL_ID ���g�p���Ĉ��S�ɍč\���ł��܂��B*/
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
	context = eglCreateContext(display, config, NULL, NULL);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	engine->display = display;
	engine->context = context;
	engine->surface = surface;
	engine->width = w;
	engine->height = h;

	// GL �̏�Ԃ����������܂��B
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_DEPTH_TEST);

	return 0;
}

/**
* �f�B�X�v���C���̌��݂̃t���[���̂݁B
*/
static void engine_draw_frame(struct engine* engine) {
	if (engine->display == NULL) {
		// �f�B�X�v���C������܂���B
		return;
	}
	engine->state.FFTUnit.OutputDisp(&(engine->display),&(engine->surface));
}

/**
* ���݃f�B�X�v���C�Ɋ֘A�t�����Ă��� EGL �R���e�L�X�g���폜���܂��B
*/
static void engine_term_display(struct engine* engine) {
	if (engine->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (engine->context != EGL_NO_CONTEXT) {
			eglDestroyContext(engine->display, engine->context);
		}
		if (engine->surface != EGL_NO_SURFACE) {
			eglDestroySurface(engine->display, engine->surface);
		}
		eglTerminate(engine->display);
	}
	engine->animating = 0;
	engine->display = EGL_NO_DISPLAY;
	engine->context = EGL_NO_CONTEXT;
	engine->surface = EGL_NO_SURFACE;
}

/**
* ���̓��̓C�x���g���������܂��B
*/
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
	struct engine* engine = (struct engine*)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		engine->state.x = AMotionEvent_getX(event, 0);
		engine->state.y = AMotionEvent_getY(event, 0);
		return 1;
	}
	return 0;
}

/**
* ���̃��C�� �R�}���h���������܂��B
*/
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
	struct engine* engine = (struct engine*)app->userData;
	switch (cmd) {
	case APP_CMD_SAVE_STATE:
		// ���݂̏�Ԃ�ۑ�����悤�V�X�e���ɂ���ėv������܂����B�ۑ����Ă��������B
		engine->app->savedState = malloc(sizeof(struct saved_state));
		*((struct saved_state*)engine->app->savedState) = engine->state;
		engine->app->savedStateSize = sizeof(struct saved_state);
		break;
	case APP_CMD_INIT_WINDOW:
		// �E�B���h�E���\������Ă��܂��B�������Ă��������B
		if (engine->app->window != NULL) {
			engine_init_display(engine);
			engine_draw_frame(engine);
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// �E�B���h�E����\���܂��͕��Ă��܂��B�N���[�� �A�b�v���Ă��������B
		engine_term_display(engine);
		break;
	case APP_CMD_GAINED_FOCUS:
		// �A�v�����t�H�[�J�X���擾����ƁA�����x�v�̊Ď����J�n���܂��B
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_enableSensor(engine->sensorEventQueue,engine->accelerometerSensor);
			ASensorEventQueue_setEventRate(engine->sensorEventQueue,engine->accelerometerSensor,  1000);
			engine_draw_frame(engine);
		}
		break;
	case APP_CMD_LOST_FOCUS:
		// �A�v�����t�H�[�J�X�������ƁA�����x�v�̊Ď����~���܂��B
		// ����ɂ��A�g�p���Ă��Ȃ��Ƃ��̃o�b�e���[��ߖ�ł��܂��B
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_disableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
		}
		// �܂��A�A�j���[�V�������~���܂��B
		engine->animating = 0;
		engine_draw_frame(engine);
		break;
	}
}

/**
* ����́Aandroid_native_app_glue ���g�p���Ă���l�C�e�B�u �A�v���P�[�V����
* �̃��C�� �G���g�� �|�C���g�ł��B���ꎩ�̂̃X���b�h�ŃC�x���g ���[�v�ɂ���Ď��s����A
* ���̓C�x���g���󂯎�����葼�̑�������s�����肵�܂��B
*/
void android_main(struct android_app* state) {
	struct engine engine;

	memset(&engine, 0, sizeof(engine));
	state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	engine.app = state;
	// �����x�v�̊Ď��̏���
	engine.sensorManager = ASensorManager_getInstance();
	engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,ASENSOR_TYPE_ACCELEROMETER);
	engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,state->looper, LOOPER_ID_USER, NULL, NULL);
	ASensorEventQueue_setEventRate(engine.sensorEventQueue,engine.accelerometerSensor, (1000L));
	engine.state.FFTUnit.InitFFT("mnt/sdcard/RF.csv", "mnt/sdcard/FFTRF.csv");
	engine.state.FFTUnitTest.InitFFT("mnt/sdcard/MF.csv", "mnt/sdcard/FFTMF.csv");
	if (state->savedState != NULL) {
		// �ȑO�̕ۑ���ԂŊJ�n���܂��B�������Ă��������B
		engine.state = *(struct saved_state*)state->savedState;
	}
	ASensorEvent event;
	if (engine.accelerometerSensor != NULL) {

		while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &event, 1) > 0) {
			engine.state.FFTUnit.SetAxel(&event);
			engine.state.FFTUnitTest.GenerateFreq(&event);
		}
		engine.state.FFTUnit.TryProcess();
		engine.state.FFTUnitTest.TryProcess();
	}
	engine.animating =1;
	// ���[�v�̓X�^�b�t�ɂ��J�n��҂��Ă��܂��B
	while (1) {
		// �ۗ����̂��ׂẴC�x���g��ǂݎ��܂��B
		int ident;
		int events;
		struct android_poll_source* source;

		// �A�j���[�V�������Ȃ��ꍇ�A�������Ƀu���b�N���ăC�x���g����������̂�҂��܂��B
		// �A�j���[�V��������ꍇ�A���ׂẴC�x���g���ǂݎ����܂Ń��[�v���Ă��瑱�s���܂�
		// �A�j���[�V�����̎��̃t���[����`�悵�܂��B
		while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,(void**)&source)) >= 0) {

			// ���̃C�x���g���������܂��B
			if (source != NULL) {
				source->process(state, source);
			}
			// �Z���T�[�Ƀf�[�^������ꍇ�A�������������܂��B
			if (ident == LOOPER_ID_USER) {
				if (engine.accelerometerSensor != NULL) {
					while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &event, 1) > 0) {
						engine.state.FFTUnit.SetAxel(&event);
						engine.state.FFTUnitTest.GenerateFreq(&event);
					}
					engine.state.FFTUnit.TryProcess();
					engine.state.FFTUnitTest.TryProcess();
					engine.state.FFTUnit.OutputDisp(&engine.display, &engine.surface);
				}
			}
			// �I�����邩�ǂ����m�F���܂��B
			if (state->destroyRequested != 0) {
				engine_term_display(&engine);
				return;
			}
		}
		// ��ʂ̕`��(60Hz)
		if (engine.animating) {
			engine_draw_frame(&engine);
		}
	}
}
