#include "SCP/ClientGUI/App.h"

namespace SCP::ClientGUI
{
    bool SCPApp::OnInit()
    {
        MainUI* mainui = new MainUI;
        mainui->Show();
        return true;
    }
}

wxIMPLEMENT_APP(SCP::ClientGUI::SCPApp);