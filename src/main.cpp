#include "main.h"
#include "config.h"
#include "log.h"
#include <mpi.h>

#if ENABLE_TUIO_TOUCH_LISTENER
    #include "TouchListener.h"
    #include <X11/Xlib.h>
#endif

QApplication * g_app = NULL;
int g_mpiRank = -1;
int g_mpiSize = -1;
MPI_Comm g_mpiRenderComm;
Configuration * g_configuration = NULL;
boost::shared_ptr<DisplayGroup> g_displayGroup;
MainWindow * g_mainWindow = NULL;
PixelStreamSourceListener * g_pixelStreamSourceListener = NULL;
long g_frameCount = 0;

int main(int argc, char * argv[])
{
    put_flog(LOG_INFO, "");

#if ENABLE_TUIO_TOUCH_LISTENER
    // we need X multithreading support if we're running the TouchListener thread and creating X events
    XInitThreads();
#endif

    g_app = new QApplication(argc, argv);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &g_mpiRank);
    MPI_Comm_size(MPI_COMM_WORLD, &g_mpiSize);
    MPI_Comm_split(MPI_COMM_WORLD, g_mpiRank != 0, g_mpiRank, &g_mpiRenderComm);

    g_configuration = new Configuration("configuration.xml");

    boost::shared_ptr<DisplayGroup> dg(new DisplayGroup);
    g_displayGroup = dg;

    g_mainWindow = new MainWindow();

#if ENABLE_TUIO_TOUCH_LISTENER
    if(g_mpiRank == 0)
    {
        new TouchListener();
    }
#endif

    if(g_mpiRank == 0)
    {
        g_pixelStreamSourceListener = new PixelStreamSourceListener();
    }

    // enter Qt event loop
    g_app->exec();

    put_flog(LOG_DEBUG, "quitting");

    if(g_mpiRank == 0)
    {
        g_displayGroup->sendQuit();
    }

    // clean up the MPI environment after the Qt event loop exits
    MPI_Finalize();
}
