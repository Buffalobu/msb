#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QVector>
#include <vector>
#include <QByteArray>

#include "msb/PluginTools/PluginCallTool.h"
//#include <boost/function.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//    kome::objects::Parameters *p = new kome::objects::Parameters;
//    kome::plugin::PluginCallTool::setPath(*p, "E:\\data");
//    openMsb(p);
//    kome::objects::Parameters* params = new kome::objects::Parameters;
//    saveMsb(params );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_UI_PB_ReadMSB_clicked()
{
    kome::io::msb::MsbSampleSet* msbSampleSet = new kome::io::msb::MsbSampleSet();
    //raw data file.
//    std::string sourceFilePath = "G:\\QT\\qt_msb_test\\cMsb\\msb\\sampleData\\001.msb";//this should be absolute path. cuz "absolutepath" function is invalid.
//    std::string sourceFilePath = "G:\\QT\\qt_msb_test\\cMsb\\msb\\sampleData\\bsa0.msb";
    std::string sourceFilePath = "G:\\QT\\qt_msb_test\\cMsb\\msb\\sampleData\\bsa1.msb";
    msbSampleSet->setSourceFilePath(sourceFilePath.c_str());
//    msbSampleSet->openFile(sourceFilePath.c_str());

    kome::io::msb::MsbSample* msbSample = new kome::io::msb::MsbSample(msbSampleSet);

    kome::io::msb::MsbIO0202 msb0202;
    msb0202.readSampleInfo(*msbSample);
    msb0202.readMsb(*msbSample);
}




//转换
void MainWindow::on_UI_PB_Convert_clicked()
{
    //读硅油文件



    //写msb文件
}

//制作parameters
kome::objects::Variant MainWindow::make_params(kome::objects::Parameters &params)
{
    // return value
    kome::objects::Variant ret;
    ret.type = kome::objects::Variant::BOOL;

    // creats dataSet object
    kome::objects::DataSet* dataSet = new kome::objects::DataSet();
    // sets dataSet values

    std::string sampleSetFilePath = "";
    dataSet->getSample()->getSampleSet()->setFilePath(sampleSetFilePath.c_str());
    std::string sampleName = "";
    dataSet->getSample()->setName(sampleName.c_str());
    std::string instrumentName = "YuDa-MS";
    dataSet->getSample()->setInstrument(instrumentName.c_str());
    std::string manufacture = "YuDa Company";
    dataSet->getSample()->setMsCompany(manufacture.c_str());
    std::string softwareName = "OMS";
    dataSet->getSample()->setSoftwareName(softwareName.c_str());
    std::string softwareVersion = "V1.0";
    dataSet->getSample()->setSoftwareVersion(softwareVersion.c_str());


    kome::io::msb::MsbSampleSet* msbSampleSet = new kome::io::msb::MsbSampleSet();
    //raw data file.
    std::string sourceFilePath = "";//this should be absolute path. cuz "absolutepath" function is invalid.
//    msbSampleSet->setSourceFilePath(sourceFilePath.c_str());
    msbSampleSet->openFile(sourceFilePath.c_str());

    kome::io::msb::MsbSample* msbSample = new kome::io::msb::MsbSample(msbSampleSet);
    std::string name = "";
    FILE m_File;
    kome::io::msb::MsbSpectrum* msbSpec = new kome::io::msb::MsbSpectrum(msbSample, name.c_str(), &m_File);    
    dataSet->addSpectrum(msbSpec);
    //sets dataSet to params
    if( dataSet == nullptr ) {
        return ret;
    }
    kome::plugin::PluginCallTool::setDataSet(params, *dataSet);

    //set path values
    std::string path = "D:/";
    if( path.size() == 0 ) {
        return ret;
    }
    kome::plugin::PluginCallTool::setPath(params, path.c_str());

    //creats progress object
    kome::core::Progress* progress = &kome::core::Progress::getIgnoringProgress();
    //sets progress values

    //sets progress to params
//    if( progress == nullptr ) {
//        progress = &kome::core::Progress::getIgnoringProgress();
//    }
    kome::plugin::PluginCallTool::setProgress(params, *progress);

    //creats SettingParameterValues object
    kome::objects::SettingParameterValues* saveSettings = new kome::objects::SettingParameterValues();
    //sets saveSettings values
    //use setValue() method to set some parameter value.
    saveSettings->setValue("nothing", "nothing");
    //sets saveSettings to params
    kome::plugin::PluginCallTool::setSettingValues(params, *saveSettings);

    return ret;
}

void MainWindow::on_UI_PB_LOAD_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
//                                                    QCoreApplication::applicationDirPath()+ "\\data",//QDir::currentPath()
                                                    "G:\\QT\\qt_msb_test\\data",
                                                     "TIC Files(*.Param);;Temp Files(*.P)",
                                                     nullptr);
//    QString file_full = QFileDialog::getOpenFileName(this,.....);
    QFileInfo fileinfo = QFileInfo(fileName);
    //文件名
    QString file_name = fileinfo.path()+"/"+fileinfo.baseName();
    //文件后缀
    QString file_suffix = fileinfo.suffix();
    if (!fileName.isNull()){

        if(cFileData::loadFileTIC(Period, pIndexArray, pTicX, pTicY, pStreamHead, mPageTIC, fileName)){
            int a = 0;
            a++;
        }

         QString filePathD;
         filePathD= file_name + ".Dat";
         int index = 0;

         if(cFileData::loadFileMass(index, pIndexArray, mGraphBuffer, pStreamBody, filePathD)){
             cParamValue::_Segment* pSegmentLIT=nullptr;
             splitStreamHead(pStreamHead, pSegmentLIT);
             dataDismantleFirst(mGraphBuffer, pSegmentLIT, &(tmpThreadBuffX), &(tmpThreadBuffY), listSTRUCT_DATA, isRestart = false);

         }
   }
}

int MainWindow::splitStreamHead(QByteArray& pStreamHead, cParamValue::_Segment*& pSegment)
{
    QList<cParamValue::_StreamHeadParam*> tmpList;
    if(!_StreamHead::toList(pStreamHead, tmpList))
        return -1;
    //cParamValue::_EventLIT* pEventLIT=nullptr;
    for(int i=0;i<tmpList.size();++i){
        switch(tmpList[i]->type){
        case cParamValue::Type_Segment_Param:{
            mSegment.resize(tmpList[i]->length- sizeof(cParamValue::_StreamHeadParam));
            memcpy(mSegment.data(), tmpList[i]->param, mSegment.size());
            pSegment= (cParamValue::_Segment*)(mSegment.data());
            //pEventLIT= (cParamValue::_EventLIT*)&(((cParamValue::_Segment*)(tmpList[i]->param))->fisrtEvent);
            break;}
        case cParamValue::Type_Method_Param:{
            QByteArray tmpArray;
            tmpArray.resize(tmpList[i]->length- sizeof(cParamValue::_StreamHeadParam));
            memcpy(tmpArray.data(), tmpList[i]->param, tmpArray.size());
            mChartHead= QString::fromUtf8(tmpArray);
            QStringList tmpStringList= QString::fromUtf8(tmpArray).split("\n");
            if(tmpStringList.size()>1){
                for(int j=0;j<tmpStringList.size();++j){
                    if("PolyFit"== tmpStringList[j]){
                        _CONGIG_OMS::splitCalibrat(tmpStringList[j+1], mCALIBRATE);
                        break;
                    }
                }
            }
            break;}
        case cParamValue::Type_Process_Param:{
            QByteArray tmpArray;
            tmpArray.resize(tmpList[i]->length-sizeof(cParamValue::_StreamHeadParam));
            memcpy(tmpArray.data(), tmpList[i]->param, tmpArray.size());
            //QString tmpStr= QString::fromUtf8(tmpArray);
            QStringList tmpStringList= QString::fromUtf8(tmpArray).split("\n");
            if(tmpStringList.size()>1){
                for(int j=0;j<tmpStringList.size();++j){
                    if("Processing"== tmpStringList[j]){
//                        if(mDataProcess)
//                            mDataProcess->setMethod(tmpStringList[j+1]);
//                        else
//                            return 0;
                        break;
                    }
                }
            }
            break;}
        default:break;
        }
    }
}

uint32_t MainWindow::dataDismantleFirst(QByteArray& pByteArray, cParamValue::_Segment* pSegment,
                                QList<std::vector<double>>* pListX, QList<std::vector<double>>* pListY,
                                QList<_CONGIG_OMS::_STRUCT_DATA>& pSTRUCT_DATA, bool restart)
{
    if(pByteArray.isEmpty()||(pSegment==nullptr)||(pListY==nullptr))
        return 0;
    cParamValue::_EventLIT* pEventLIT= (cParamValue::_EventLIT*)&(pSegment->fisrtEvent);
    if(pEventLIT==nullptr)
        return 0;
    double dbEvtTimeSum= 0;//pEventLIT->holdTime;
    for(uint32_t i=0;i<pSegment->sumEvent;i++)
        dbEvtTimeSum+= ((cParamValue::_EventLIT*)((int*)pEventLIT+ sizeof(cParamValue::_EventLIT)/sizeof(int)*i))->holdTime;
    if(dbEvtTimeSum< 0.0000001)
        return 0;

    if((uint32_t)(pListY->size())!= pSegment->sumEvent){
        pListY->clear();
        pListX->clear();
        for(uint32_t i=0;i<pSegment->sumEvent;i++){
            pListY->append(std::vector<double>(0));
            pListX->append(std::vector<double>(0));
        }
    }
    if(restart){
        pSTRUCT_DATA.clear();
        for(uint32_t currentEvt=0;currentEvt<pSegment->sumEvent;currentEvt++){
            //_CONGIG_OMS::_STRUCT_DATA tempSTRUCT_DATA;
            pSTRUCT_DATA.append(_CONGIG_OMS::_STRUCT_DATA());
        }
    }
    uint32_t uAllPoint = pByteArray.size() / sizeof(double);
    double* pdbOffset = (double*)(pByteArray.data());
    for(uint32_t currentEvt=0;currentEvt<pSegment->sumEvent;currentEvt++){
        pEventLIT=((cParamValue::_EventLIT*)((int*)&(pSegment->fisrtEvent)+ sizeof(cParamValue::_EventLIT)/sizeof(int)*currentEvt));
        _CONGIG_OMS::_STRUCT_DATA* tempSTRUCT_DATA= &(pSTRUCT_DATA[currentEvt]);
        //tempSTRUCT_DATA->uPrePoint = (uint32_t)(pEventLIT->preReadyTime * uAllPoint /dbEvtTimeSum);
        //tempSTRUCT_DATA->uPostPoint = (uint32_t)(pEventLIT->postReadyTime * uAllPoint /dbEvtTimeSum);
        tempSTRUCT_DATA->uEvtValidPoint = (uint32_t)(pEventLIT->holdTime * uAllPoint / dbEvtTimeSum);
        //tempSTRUCT_DATA->uEventPoint= tempSTRUCT_DATA->uEvtValidPoint- tempSTRUCT_DATA->uPrePoint- tempSTRUCT_DATA->uPostPoint;

        if((!vectorOperate::Resize((*pListY)[currentEvt],tempSTRUCT_DATA->uEvtValidPoint))
            ||(!vectorOperate::Resize((*pListX)[currentEvt],tempSTRUCT_DATA->uEvtValidPoint)))
                return 0;

        if(restart){
            double* pFirst= (*pListX)[currentEvt].data();
            double* pLast= pFirst+ tempSTRUCT_DATA->uEvtValidPoint-1;
            _CONGIG_OMS::calibrationF(mCALIBRATE, pFirst, pLast, pEventLIT, tempSTRUCT_DATA);
            tempSTRUCT_DATA->uEventPoint= tempSTRUCT_DATA->uEvtValidPoint- tempSTRUCT_DATA->uPrePoint- tempSTRUCT_DATA->uPostPoint;

        }
        memcpy((*pListY)[currentEvt].data(), pdbOffset, tempSTRUCT_DATA->uEvtValidPoint* sizeof(double));
        pdbOffset += tempSTRUCT_DATA->uEvtValidPoint;
    }
    return uAllPoint;
}
