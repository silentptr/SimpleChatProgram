#include "SCP/ClientGUI/MainUI.h"

namespace SCP::ClientGUI
{
    MainUI::MainUI() :
    wxFrame(NULL, wxID_ANY, "Simple Chat Program", wxDefaultPosition, wxSize(800, 600), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX))
    {
        wxMenuBar* menuBar = new wxMenuBar;

        wxMenu* clientMenu = new wxMenu;
        clientMenu->Append(SCP_MAINUI_CONNECT, "Connect");
        clientMenu->Append(SCP_MAINUI_DISCONNECT, "Disconnect");

        menuBar->Append(clientMenu, "Client");
        SetMenuBar(menuBar);

        Bind(wxEVT_MENU, &MainUI::OnConnectButton, this, SCP_MAINUI_CONNECT);
        Bind(wxEVT_MENU, &MainUI::OnDisconnectButton, this, SCP_MAINUI_DISCONNECT);

        wxPanel* panel = new wxPanel(this);
        wxBoxSizer* vertSizer = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer* horiSizer = new wxBoxSizer(wxHORIZONTAL);
        wxFlexGridSizer* sizer = new wxFlexGridSizer(2, 2, 3, 3);
        panel->SetSizer(vertSizer);

        m_ChatBox = new wxTextCtrl(panel, wxID_ANY, "> Welcome!", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
        m_InputBox = new wxTextCtrl(panel, SCP_MAINUI_INPUTBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
        Bind(wxEVT_TEXT_ENTER, &MainUI::OnTextInput, this, SCP_MAINUI_INPUTBOX);
        m_SendButton = new wxButton(panel, SCP_MAINUI_SENDBTN, "Send");
        Bind(wxEVT_BUTTON, &MainUI::OnSendMessage, this, SCP_MAINUI_SENDBTN);

        vertSizer->Add(m_ChatBox, 1, wxEXPAND | wxALL, 6);
        horiSizer->Add(m_InputBox, 1, wxEXPAND | wxRIGHT | wxLEFT | wxDOWN, 6);
        horiSizer->Add(m_SendButton, 0, wxRIGHT | wxDOWN, 6);
        vertSizer->Add(horiSizer, 0, wxEXPAND);

        CenterOnScreen();
    }

    void MainUI::SendMessage(const std::string_view& msg)
    {

    }

    void MainUI::OnConnectButton(wxCommandEvent&)
    {
        (new ConnectDialog(this))->ShowModal();
    }

    void MainUI::OnDisconnectButton(wxCommandEvent&)
    {

    }

    void MainUI::OnTextInput(wxCommandEvent&)
    {
        SendMessage(m_InputBox->GetValue().ToStdString());
        m_InputBox->Clear();
    }

    void MainUI::OnSendMessage(wxCommandEvent&)
    {
        SendMessage(m_InputBox->GetValue().ToStdString());
        m_InputBox->Clear();
    }

    void MainUI::OnConnect(std::optional<std::string> err)
    {

    }

    void MainUI::OnChatMessage(std::string msg)
    {

    }

    void MainUI::OnDisconnect(std::optional<std::string> err)
    {

    }
}