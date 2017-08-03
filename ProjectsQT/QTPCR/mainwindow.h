#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>
#include <QMainWindow>
#include <qcustomplot.h>
#include <QtSerialPort/QSerialPort>
#include <QTimer>

QT_BEGIN_NAMESPACE

class QLabel;

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class Consola;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void Setup(QCustomPlot *customPlot);

private slots:
    void openSerialPort();
    void closeSerialPort();
    void readData();
    void handleError(QSerialPort::SerialPortError error);
    void realtimeDataSlot();
    void on_botonEnvioDatos_clicked();
    void on_ResetButton_clicked();
    void on_comboBox_activated(const QString &arg1);

private:
    void initActionsConnections();
    void startScanningOfSerialPorts();
    void ColocarClaveParaProtocolo(char clave,QByteArray *infoSerial);
    void preFillTextFields();
    void setInitialLayout();
    void setValidatorsInTextFields();
    void obtenerRangoParaGraficar(QString *datos, QString &protocolo);
    void layoutHeatBlock();
    void layoutEnfriar();
    void layoutGraphPCR();
    void layoutGraphHeatBlock();
    void layoutGraphEnfriar();
    void checkIfIncomingDataAvailable();


private:
    void showStatusMessage(const QString &message);
    Ui::MainWindow *ui;
    QLabel *status;
    QSerialPort *serial;
    QTimer dataTimer, timerForScanningCOMPorts;
    float temperaturaBase;
    float temperaturaLid;
    int tiempoTotal;
    int ciclo, escalon;
    double fechaInicio;
    bool fechaInicial = TRUE;
    QCPItemText *labelGraphRealTime, *labelGraphProtocolData;
    QDoubleValidator *validateFloat = new QDoubleValidator(0.0,10.0,1,this);
    QIntValidator *validateIntegerTime = new QIntValidator(0, 9999, this);
    QIntValidator *validateIntegerCycles = new QIntValidator(0,99,this);
};

#endif // MAINWINDOW_H
