#ifndef SCP_CLIENTGUI_APP_H_
#define SCP_CLIENTGUI_APP_H_

#include <wx/wx.h>

#include "SCP/ClientGUI/MainUI.h"

namespace SCP::ClientGUI
{
    class SCPApp : public wxApp
    {
    public:
        bool OnInit() override;
    };
}

#endif