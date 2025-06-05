#include "myncnn.h"

TcpClient* gTcpClient = nullptr;

MYNCNN::~MYNCNN()
{
    det_target_size = 640;
}

int MYNCNN::load(const char* parampath, const char* modelpath)
{
    myncnn.clear();

    myncnn.opt = ncnn::Option();

    myncnn.load_param(parampath);
    myncnn.load_model(modelpath);

    return 0;
}

int MYNCNN::load(AAssetManager* mgr, const char* parampath, const char* modelpath)
{
    myncnn.clear();

    myncnn.opt = ncnn::Option();

    myncnn.load_param(mgr, parampath);
    myncnn.load_model(mgr, modelpath);

    return 0;
}

void MYNCNN::set_det_target_size(int target_size)
{
    det_target_size = target_size;
}

void MYNCNN::set_number(int number) {
    myNumber = number;
}

int MYNCNN::get_number() {
    return myNumber;
}