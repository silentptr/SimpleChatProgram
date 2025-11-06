#include "SCP/ClientGUI/MainUI.h"

wxDEFINE_EVENT(MY_NEW_TYPE, wxCommandEvent);

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

        m_ChatBox = new wxTextCtrl(panel, wxID_ANY, "Welcome!\n", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
        m_InputBox = new wxTextCtrl(panel, SCP_MAINUI_INPUTBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
        Bind(wxEVT_TEXT_ENTER, &MainUI::OnTextInput, this, SCP_MAINUI_INPUTBOX);
        m_SendButton = new wxButton(panel, SCP_MAINUI_SENDBTN, "Send");
        Bind(wxEVT_BUTTON, &MainUI::OnSendMessage, this, SCP_MAINUI_SENDBTN);

        vertSizer->Add(m_ChatBox, 1, wxEXPAND | wxALL, 6);
        horiSizer->Add(m_InputBox, 1, wxEXPAND | wxRIGHT | wxLEFT | wxDOWN, 6);
        horiSizer->Add(m_SendButton, 0, wxRIGHT | wxDOWN, 6);
        vertSizer->Add(horiSizer, 0, wxEXPAND);

        Bind(MY_NEW_TYPE, &MainUI::OnClientConnect, this, SCP_MAINUI_EVT_CONNECT);
        Bind(MY_NEW_TYPE, &MainUI::OnClientMsg, this, SCP_MAINUI_EVT_CHATMSG);
        Bind(MY_NEW_TYPE, &MainUI::OnClientDisconnect, this, SCP_MAINUI_EVT_DISCONNECT);

        CenterOnScreen();
    }

    void MainUI::SendChatMessage(const std::string_view& msg)
    {
        SendMessage(msg);
    }

    void MainUI::OnConnectButton(wxCommandEvent&)
    {
        if (GetState() == SCP::Client::ChatClientState::Connecting)
        {
            wxMessageBox("already connecting", "SCP", 5L, this);
            return;
        }
        
        ConnectDialog* dialog = new ConnectDialog(this);
        dialog->ShowModal();
        auto connectInfo = dialog->GetConnectInfo();

        if (connectInfo)
        {
            ChatClient::Disconnect();
            m_ChatBox->AppendText("Connecting to " + connectInfo->m_IP + ":" + std::to_string(connectInfo->m_Port) + " with the username "
            + connectInfo->m_Username + "...\n");
            ChatClient::Connect(connectInfo->m_IP, connectInfo->m_Port, connectInfo->m_Username);
        }
    }

    void MainUI::OnDisconnectButton(wxCommandEvent&)
    {
        ChatClient::Disconnect();
    }

    void MainUI::OnTextInput(wxCommandEvent&)
    {
        SendChatMessage(m_InputBox->GetValue().ToStdString());
        m_InputBox->Clear();
    }

    void MainUI::OnSendMessage(wxCommandEvent&)
    {
        SendChatMessage(m_InputBox->GetValue().ToStdString());
        m_InputBox->Clear();
    }

    void MainUI::OnClientConnect(wxCommandEvent& e)
    {
        if (e.GetInt())
        {
            m_ChatBox->AppendText(e.GetString() + "\n");
        }
    }

    void MainUI::OnClientMsg(wxCommandEvent& e)
    {
        m_ChatBox->AppendText(e.GetString() + "\n");
    }

    void MainUI::OnClientDisconnect(wxCommandEvent& e)
    {
        m_ChatBox->AppendText(e.GetString() + "\n");
    }

    void MainUI::OnConnect(std::optional<std::string> err)
    {
        wxCommandEvent* e = new wxCommandEvent(MY_NEW_TYPE, SCP_MAINUI_EVT_CONNECT);
        e->SetInt(err.has_value());

        if (err)
        {
            e->SetString("Could not connect to server: " + std::string((*err).c_str()));
        }

        wxQueueEvent(this, e);
    }

    void MainUI::OnMessage(std::string msg)
    {
        wxCommandEvent* e = new wxCommandEvent(MY_NEW_TYPE, SCP_MAINUI_EVT_CHATMSG);
        e->SetString(msg.c_str());
        wxQueueEvent(this, e);
    }

    void MainUI::OnDisconnect(std::optional<std::string> err)
    {
        wxCommandEvent* e = new wxCommandEvent(MY_NEW_TYPE, SCP_MAINUI_EVT_DISCONNECT);

        if (err)
        {
            e->SetString("Disconnected from server with error: " + std::string((*err).c_str()));
        }
        else
        {
            e->SetString("Disconnected from server");
        }
        
        wxQueueEvent(this, e);
    }
}