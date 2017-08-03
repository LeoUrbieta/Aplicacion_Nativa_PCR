#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QLabel>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setInitialLayout();
    preFillTextFields();
    setValidatorsInTextFields();
    setGeometry(400, 250, 1000, 1000);
    serial = new QSerialPort(this);
    status = new QLabel;

    ui->statusBar->addWidget(status);

    startScanningOfSerialPorts();

    connect(serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &MainWindow::handleError);

    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);

}

void MainWindow::startScanningOfSerialPorts(){

    connect(&timerForScanningCOMPorts,SIGNAL(timeout()),this,SLOT(openSerialPort()));
    timerForScanningCOMPorts.start(1000);
}

void MainWindow::setValidatorsInTextFields(){
    validateFloat->setNotation(QDoubleValidator::StandardNotation);
    ui->InputTempAnneal->setValidator( validateFloat );
    ui->InputTempDenat->setValidator(validateFloat);
    ui->InputTempPrimer->setValidator(validateFloat);
    ui->InitDenatTime->setValidator(validateIntegerTime);
    ui->DenatTime->setValidator(validateIntegerTime);
    ui->PrimerTime->setValidator(validateIntegerTime);
    ui->AnnealTime->setValidator(validateIntegerTime);
    ui->FinalExtensionTime->setValidator(validateIntegerTime);
    ui->InputNumCiclos->setValidator(validateIntegerCycles);
    ui->InputTempHeatBlock->setValidator(validateFloat);
    ui->InputTiempoHeatBlock->setValidator(validateIntegerTime);
}

void MainWindow::setInitialLayout(){
    ui->customPlot->hide();
    ui->labelImage->show();
    ui->ResetButton->hide();
    ui->progressBar->hide();
    ui->HeatBlockBox->hide();
    ui->groupBox_2->show();
    ui->botonEnvioDatos->show();
    ui->LowTempBox->hide();
    ui->comboBox->show();
    ui->labelHeatBlock->hide();
    ui->labelEnfriar->hide();
}

void MainWindow::preFillTextFields(){
    ui->InputTempAnneal->setText("72.0");
    ui->InputTempDenat->setText("95.0");
    ui->InputTempPrimer->setText("60.0");
    ui->InitDenatTime->setText("30");
    ui->DenatTime->setText("30");
    ui->PrimerTime->setText("30");
    ui->AnnealTime->setText("30");;
    ui->FinalExtensionTime->setText("30");
    ui->InputNumCiclos->setText("1");
    ui->InputTempHeatBlock->setText("95.0");
    ui->InputTiempoHeatBlock->setText("120");
}

void MainWindow::realtimeDataSlot()
{
    // calculate two new data points:
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
    double key = 0;
#else
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    if(fechaInicial){
        fechaInicio = key;
        fechaInicial = FALSE;
        ui->progressBar->setMinimum((int)fechaInicio);
        ui->progressBar->setMaximum((int)fechaInicio + tiempoTotal);
        temperaturaLid = 0;
        temperaturaBase = 0;
        ciclo = 0;
        escalon = -1;
    }
#endif
    static double lastPointKey = 0;

    if (key-lastPointKey > 1.0) // at most add point every 10 ms
    {
        //update labels that show info of PCR cycle
        QString tempGraphBase, tempGraphLid, escalonActual,cicloActual, protocoloEnProceso, protocolDataPCR, protocolDataHeatBlock;
        protocolDataPCR = "Time Init Denat: " + ui->InitDenatTime->text() + "sec\n" +
                          "Denaturation: " + ui->InputTempDenat->text() + "°C" + " --> " + "Time: " + ui->DenatTime->text() + "sec\n" +
                          "Annealing: " + ui->InputTempPrimer->text() +  "°C" + " -->  " + "Time: " + ui->PrimerTime->text() + "sec\n" +
                          "Extension: " + ui->InputTempAnneal->text() +  "°C" + " -->  " + "Time: " + ui->AnnealTime->text() + "sec\n" +
                          "Time Final Extension: " + ui->FinalExtensionTime->text() + "sec\n"
                          "Number of Cycles: " + ui->InputNumCiclos->text();
        protocolDataHeatBlock = "Heat Block Temperature: " + ui->InputTempHeatBlock->text() + "°C\n" +
                                "Time: " + ui->InputTiempoHeatBlock->text() + "sec";
        tempGraphLid.setNum(temperaturaLid,'f',1);
        tempGraphBase.setNum(temperaturaBase,'f',1);
        cicloActual.setNum(ciclo);
        if(escalon == 0){
            escalonActual = "Desnaturalización inicial";
        } else if(escalon == 1){
            escalonActual = "Desnaturalización";
        } else if(escalon == 2){
            escalonActual = "Alineación";
        } else if(escalon == 3){
            escalonActual = "Extensión";
        } else if(escalon == 4){
            escalonActual = "Extension Final";
        }

        protocoloEnProceso = ui->comboBox->currentText();
        if(protocoloEnProceso == "PCR"){
            labelGraphRealTime->position->setCoords(fechaInicio + tiempoTotal/2,106);
            labelGraphProtocolData->position->setCoords(fechaInicio + tiempoTotal / 7, 15 );
            labelGraphProtocolData->setText("");
            labelGraphProtocolData->setText(protocolDataPCR);
            if(escalon > 0 && escalon < 4){
                labelGraphRealTime->setText("");
                labelGraphRealTime->setText("Temperatura Base: " + tempGraphBase + " " + "Temperatura Lid: " + tempGraphLid + " " + "Ciclo: " + cicloActual + " " + "Proceso: " + escalonActual);
            } else if(escalon == 0 || escalon == 4){
                labelGraphRealTime->setText("");
                labelGraphRealTime->setText("Temperatura Base: " + tempGraphBase + " " + "Temperatura Lid: " + tempGraphLid  + " " + "Proceso: " + escalonActual);
            } else{
                labelGraphRealTime->setText("");
                labelGraphRealTime->setText("Temperatura Base: " + tempGraphBase + " " + "Temperatura Lid: " + tempGraphLid);
            }
        } else if(protocoloEnProceso == "Heat Block"){
            labelGraphRealTime->position->setCoords(fechaInicio + tiempoTotal/2,106);
            labelGraphProtocolData->position->setCoords(fechaInicio + tiempoTotal / 7, 15 );
            labelGraphRealTime->setText("");
            labelGraphRealTime->setText("Temperatura Base: " + tempGraphBase + " " + "Temperatura Lid: " + tempGraphLid);
            labelGraphProtocolData->setText("");
            labelGraphProtocolData->setText(protocolDataHeatBlock);
        } else if(protocoloEnProceso == "Mantener a 4°C"){
            labelGraphRealTime->position->setCoords(fechaInicio + tiempoTotal/2,106);
            labelGraphRealTime->setText("");
            labelGraphRealTime->setText("Temperatura Base: " + tempGraphBase + " " + "Temperatura Lid: " + tempGraphLid);
        }

        ui->progressBar->setValue((int)key);

        float value0 = temperaturaBase; //qSin(key*1.6+qCos(key*1.7)*2)*10 + qSin(key*1.2+0.56)*20 + 26;
        float value1 = temperaturaLid; //qSin(key*1.3+qCos(key*1.2)*1.2)*7 + qSin(key*0.9+0.26)*24 + 26;
        // add data to lines:
        ui->customPlot->graph(0)->addData(key, value0);
        ui->customPlot->graph(1)->addData(key, value1);
        // set data of dots:
        ui->customPlot->graph(2)->clearData();
        ui->customPlot->graph(2)->addData(key, value0);
        ui->customPlot->graph(3)->clearData();
        ui->customPlot->graph(3)->addData(key, value1);
        // remove data of lines that's outside visible range:
        //    ui->customPlot->graph(0)->removeDataBefore(key-8);
        //   ui->customPlot->graph(1)->removeDataBefore(key-8);
        // rescale value (vertical) axis to fit the current data:
        //ui->customPlot->graph(0)->rescaleValueAxis();
        //ui->customPlot->graph(1)->rescaleValueAxis(true);
        lastPointKey = key;
    }
    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlot->xAxis->setRange(fechaInicio,tiempoTotal ,Qt::AlignLeft);//setRange(key+0.25, 8, Qt::AlignRight);
    ui->customPlot->replot();

    // calculate frames per second:
    //    static double lastFpsKey;
    //    static int frameCount;
    //    ++frameCount;
    //    if (key-lastFpsKey > 2) // average fps over 2 seconds
    //    {
    //        ui->statusBar->showMessage(
    //                    QString("%1 FPS, Total Data points: %2")
    //                    .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
    //                    .arg(ui->customPlot->graph(0)->data()->count()+ui->customPlot->graph(1)->data()->count())
    //                    , 0);
    //        lastFpsKey = key;
    //        frameCount = 0;
    //    }
}

void MainWindow::Setup(QCustomPlot *customPlot){
    customPlot->show();
    customPlot->addGraph();
    //customPlot->setBackground(QBrush(QColor(0,0,0)));//119,136,153
    // blue line
    customPlot->graph(0)->setPen(QPen(Qt::green));
    // customPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
    //customPlot->graph(0)->setAntialiasedFill(false);
    customPlot->addGraph(); // red line
    customPlot->graph(1)->setPen(QPen(Qt::red));
    // customPlot->graph(0)->setChannelFillGraph(customPlot->graph(1));

    customPlot->addGraph(); // blue dot
    customPlot->graph(2)->setPen(QPen(Qt::green));
    customPlot->graph(2)->setLineStyle(QCPGraph::lsNone);
    customPlot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
    customPlot->addGraph(); // red dot
    customPlot->graph(3)->setPen(QPen(Qt::red));
    customPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
    customPlot->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);

    customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
    customPlot->xAxis->setAutoTickStep(TRUE);
    customPlot->xAxis->setBasePen(QPen(Qt::white, 1));
    customPlot->yAxis->setBasePen(QPen(Qt::white, 1));
    customPlot->xAxis->setTickPen(QPen(Qt::white, 1));
    customPlot->yAxis->setTickPen(QPen(Qt::white, 1));
    customPlot->xAxis->setSubTickPen(QPen(Qt::white, 1));
    customPlot->yAxis->setSubTickPen(QPen(Qt::white, 1));
    customPlot->xAxis->setTickLabelColor(Qt::white);
    customPlot->yAxis->setTickLabelColor(Qt::white);
    customPlot->xAxis2->setBasePen(QPen(Qt::white, 1));
    customPlot->yAxis2->setBasePen(QPen(Qt::white, 1));
    customPlot->xAxis2->setTickPen(QPen(Qt::white, 1));
    customPlot->yAxis2->setTickPen(QPen(Qt::white, 1));
    customPlot->xAxis2->setSubTickPen(QPen(Qt::white, 1));
    customPlot->yAxis2->setSubTickPen(QPen(Qt::white, 1));
    customPlot->xAxis2->setTickLabelColor(Qt::white);
    customPlot->yAxis2->setTickLabelColor(Qt::white);

    customPlot->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    customPlot->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    customPlot->xAxis->grid()->setSubGridVisible(true);
    customPlot->yAxis->grid()->setSubGridVisible(true);
    customPlot->xAxis->grid()->setZeroLinePen(Qt::NoPen);
    customPlot->yAxis->grid()->setZeroLinePen(Qt::NoPen);

    QLinearGradient plotGradient;
    plotGradient.setStart(0, 0);
    plotGradient.setFinalStop(0, 350);
    plotGradient.setColorAt(0, QColor(80, 80, 80));
    plotGradient.setColorAt(1, QColor(50, 50, 50));
    customPlot->setBackground(plotGradient);

    labelGraphRealTime = new QCPItemText(customPlot);
    labelGraphRealTime->setFont(QFont(font().family(), 12));
    labelGraphRealTime->setColor(Qt::white);

    labelGraphProtocolData = new QCPItemText(customPlot);
    labelGraphProtocolData->setFont(QFont(font().family(), 12));
    labelGraphProtocolData->setColor(Qt::white);
    labelGraphProtocolData->setTextAlignment(Qt::AlignLeft);

    //customPlot->xAxis->setTickStep(2);
    customPlot->yAxis->setRange(0,110);
    customPlot->axisRect()->setupFullAxesBox();

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(0); // Interval 0 means to refresh as fast as possible
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openSerialPort()
{
    QList<QSerialPortInfo> lista;
    QString descripcion;

    lista = QSerialPortInfo::availablePorts();

    for(int i = 0; i < QSerialPortInfo::availablePorts().count(); i++){
        descripcion = lista[i].description();
        if(descripcion.contains("CP2105") && descripcion.contains("Standard")){
            serial->setPort(lista[i]);
        }
    }
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    if(serial->open(QIODevice::ReadWrite)){
        showStatusMessage(tr("Conectado"));
        timerForScanningCOMPorts.stop();
        disconnect(&timerForScanningCOMPorts,SIGNAL(timeout()),this,SLOT(openSerialPort()));
    }

}

void MainWindow::closeSerialPort()
{
    if(serial->isOpen()){
        serial->close();
        connect(&timerForScanningCOMPorts,SIGNAL(timeout()),this,SLOT(openSerialPort()));
        timerForScanningCOMPorts.start(1000);
    }
    showStatusMessage(tr("Desconectado"));
}

void MainWindow::readData()
{
    QString data;

    while(serial->canReadLine()){

        data = serial->readLine();

    }

    QStringList temperatures = data.split(' ');

    double tiempoActual = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;

    static double tiempoPasado = 0;
    if(tiempoActual - tiempoPasado > 1.0){
        temperaturaBase = QString(temperatures[0]).toFloat();
        temperaturaLid = QString(temperatures[1]).toFloat();
        ciclo = QString(temperatures[2]).toInt();
        escalon = QString(temperatures[3]).toInt();
        tiempoPasado = tiempoActual;
    }
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if(error == QSerialPort::ResourceError){
        //QMessageBox::critical(this, tr("Parece que hay un error"), serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::obtenerRangoParaGraficar(QString *datos, QString &protocolo){

    if(protocolo == "PCR"){
        int tiempoPorCiclo = 0;
        int tiempoHeatingLid = 164;
        int tiempofirstDenaturationHeating = 46;
        int tiempoEnfriamiento = 50;
        int tiempoHeatingAExtension = 12;
        int tiempoHeatingADenaturation = 30;
        int tiempoMuertoPorCiclo = tiempoEnfriamiento + tiempoHeatingAExtension + tiempoHeatingADenaturation;
        int tiempoInitialDenaturation = datos[3].toInt();
        int tiempoFinalExtension = datos[7].toInt();

        for(int i = 4 ; i <= 6 ; i++){
            tiempoPorCiclo = tiempoPorCiclo + datos[i].toInt();
        }
        tiempoTotal = tiempoHeatingLid + tiempofirstDenaturationHeating +  tiempoInitialDenaturation + (tiempoPorCiclo + tiempoMuertoPorCiclo )* datos[8].toInt()
                + tiempoFinalExtension;

    } else if( protocolo == "Heat Block"){
        int tiempoHeatingLid = 164;
        int tiempofirstDenaturationHeating = 46;
        int tiempoHeatBlock = datos[1].toInt();

        tiempoTotal = tiempoHeatingLid + tiempofirstDenaturationHeating + tiempoHeatBlock;
    } else if(protocolo == "Mantener a 4°C"){
        tiempoTotal = 600;
    }
}

void MainWindow::showStatusMessage(const QString &message)
{
    status ->setText(message);
}

void MainWindow::ColocarClaveParaProtocolo(char clave, QByteArray *infoSerial){

    infoSerial->append(clave);
    infoSerial->append(" ");
}

void MainWindow::on_botonEnvioDatos_clicked()
{
    QString protocolo = ui->comboBox->currentText();

    if(protocolo == "PCR"){

        char claveProtocoloPCR = '0';
        int numeroVariables = 9;
        QString infoCiclo[numeroVariables];
        QByteArray infoSerial;
        int indexDecimal;

        infoCiclo[0] = ui->InputTempDenat->text();
        infoCiclo[1] = ui->InputTempPrimer->text();
        infoCiclo[2] = ui->InputTempAnneal->text();
        infoCiclo[3] = ui->InitDenatTime->text();
        infoCiclo[4] = ui->DenatTime->text();
        infoCiclo[5] = ui->PrimerTime->text();
        infoCiclo[6] = ui->AnnealTime->text();
        infoCiclo[7] = ui->FinalExtensionTime->text();
        infoCiclo[8] = ui->InputNumCiclos->text();

        for(int i = 0; i<= numeroVariables-1; i++){
            if(infoCiclo[i] == ""){
                QMessageBox::information( this, tr("MxPCR"), tr("Ups...solo para decirte que ningún campo puede estar vacío. Bye.") );
                return;
            }
        }

        obtenerRangoParaGraficar(infoCiclo,protocolo);
        ColocarClaveParaProtocolo(claveProtocoloPCR,&infoSerial);

        for(int i = 0; i <= 2 ; i++){
            if(infoCiclo[i].contains(".")){
                indexDecimal = infoCiclo[i].indexOf(".");
                if(indexDecimal == 0){
                    QMessageBox::information( this, tr("MxPCR"), tr("Las temperaturas introducidas no pueden empezar con '.'") );
                    return;
                }
                if(indexDecimal == infoCiclo[i].size() - 1){
                    infoCiclo[i].append("0");
                }
            } else {
                infoCiclo[i].append(".0");
            }
            infoCiclo[i].replace(QString("."),QString(" "));
            infoSerial.append(infoCiclo[i].toUtf8());
            infoSerial.append(" ");
        }

        for(int i = 3; i <= 7; i++){
            infoSerial.append(infoCiclo[i].toUtf8());
            infoSerial.append(" ");
        }

        infoSerial.append(infoCiclo[8].toUtf8());
        infoSerial.append(char(13));
        infoSerial.append(char(10));
        if(serial->isOpen()){
            serial->write(infoSerial,40);
            Setup(ui->customPlot);
            layoutGraphPCR();
        } else{
            QMessageBox::information(this,tr("Sin conexión"),tr("No parece haber ningún MxPCR conectado"));
            return;
        }
    } else if(protocolo == "Heat Block"){

        char claveProtocoloHeatBlock = '1';
        int numeroVariables = 2;
        QString infoCiclo[numeroVariables];
        QByteArray infoSerial;
        int indexDecimal;

        infoCiclo[0] = ui->InputTempHeatBlock->text();
        infoCiclo[1] = ui->InputTiempoHeatBlock->text();

        for(int i = 0; i<= numeroVariables-1; i++){
            if(infoCiclo[i] == ""){
                QMessageBox::information( this, tr("MxPCR"), tr("Ups...solo para decirte que ningún campo puede estar vacío. Bye.") );
                return;
            }
        }

        obtenerRangoParaGraficar(infoCiclo,protocolo);
        ColocarClaveParaProtocolo(claveProtocoloHeatBlock,&infoSerial);

        for(int i = 0; i <= 0 ; i++){
            if(infoCiclo[i].contains(".")){
                indexDecimal = infoCiclo[i].indexOf(".");
                if(indexDecimal == 0){
                    QMessageBox::information( this, tr("MxPCR"), tr("Las temperaturas introducidas no pueden empezar con '.'"));
                    return;
                }
                if(indexDecimal == infoCiclo[i].size() - 1){
                    infoCiclo[i].append("0");
                }
            } else {
                infoCiclo[i].append(".0");
            }
            infoCiclo[i].replace(QString("."),QString(" "));
            infoSerial.append(infoCiclo[i].toUtf8());
            infoSerial.append(" ");
        }

        for(int i = 1; i <= 1; i++){
            infoSerial.append(infoCiclo[i].toUtf8());
        }
        infoSerial.append(char(13));
        infoSerial.append(char(10));
        if(serial->isOpen()){
            serial->write(infoSerial,40);
            Setup(ui->customPlot);
            layoutGraphHeatBlock();
        } else{
            QMessageBox::information(this,tr("Sin conexión"),tr("No parece haber ningún MxPCR conectado"));
            return;
        }
    } else if(protocolo == "Mantener a 4°C"){

        int numeroVariables = 1;
        QString infoCiclo[numeroVariables];
        char claveProtocoloEnfriar = '2';
        QByteArray enfriarKey;

        obtenerRangoParaGraficar(infoCiclo,protocolo);
        ColocarClaveParaProtocolo(claveProtocoloEnfriar,&enfriarKey);

        enfriarKey.append(char(13));
        enfriarKey.append(char(10));
        if(serial->isOpen()){
            serial->write(enfriarKey,10);
            Setup(ui->customPlot);
            layoutGraphEnfriar();
        } else {
            QMessageBox::information(this,tr("Sin conexión"),tr("No parece haber ningún MxPCR conectado"));
            return;
        }
    }
}

void MainWindow::on_ResetButton_clicked()
{
    QString protocoloEjecutado;
    const char resetKey = 'B';
    if(serial->isOpen()){
        serial->write(&resetKey, 4);
        ui->customPlot->clearGraphs();
        labelGraphRealTime->deleteLater();
        labelGraphProtocolData->deleteLater();


        protocoloEjecutado = ui->comboBox->currentText();
        if(protocoloEjecutado == "PCR"){
            setInitialLayout();
        } else if(protocoloEjecutado == "Heat Block"){
            layoutHeatBlock();
        } else if(protocoloEjecutado == "Mantener a 4°C"){
            layoutEnfriar();
        }
        dataTimer.stop();
        disconnect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
        fechaInicial = TRUE;
    }
}

void MainWindow::layoutHeatBlock(){
    setInitialLayout();
    ui->groupBox_2->hide();
    ui->LowTempBox->hide();
    ui->labelImage->hide();
    ui->labelHeatBlock->show();
    ui->HeatBlockBox->show();
}

void MainWindow::layoutEnfriar(){
    setInitialLayout();
    ui->groupBox_2->hide();
    ui->HeatBlockBox->hide();
    ui->labelImage->hide();
    ui->LowTempBox->show();
    ui->labelEnfriar->show();
}

void MainWindow::layoutGraphPCR(){
    ui->ResetButton->show();
    ui->progressBar->show();
    ui->groupBox_2->hide();
    ui->botonEnvioDatos->hide();
    ui->comboBox->hide();
    ui->labelImage->hide();
}

void MainWindow::layoutGraphHeatBlock(){
    ui->ResetButton->show();
    ui->progressBar->show();
    ui->groupBox_2->hide();
    ui->botonEnvioDatos->hide();
    ui->comboBox->hide();
    ui->labelHeatBlock->hide();
    ui->HeatBlockBox->hide();
}

void MainWindow::layoutGraphEnfriar(){
    ui->ResetButton->show();
    ui->progressBar->show();
    ui->groupBox_2->hide();
    ui->botonEnvioDatos->hide();
    ui->comboBox->hide();
    ui->labelEnfriar->hide();
    ui->LowTempBox->hide();
}

void MainWindow::on_comboBox_activated(const QString &arg1)
{
    if(arg1 == "PCR"){
        setInitialLayout();
    } else if (arg1 == "Heat Block") {
        layoutHeatBlock();
    } else if( arg1 == "Mantener a 4°C"){
        layoutEnfriar();
    }
}
