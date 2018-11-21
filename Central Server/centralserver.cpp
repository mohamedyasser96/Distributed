#include "centralserver.h"
#include <unordered_map>
CentralServer::CentralServer(int port):UDPSocket(port)
{
    input.open(filepath.c_str());
    if (input.fail())
    {
        std::cout << "Failed to open users.txt\n";
        exit(1);
    }
    init();
}
void CentralServer::init()
{
    input.seekg(0, std::ios::beg);
    while (!input.eof())
    {
        str username;
        str password;
        input >> username;
        if (username == "") continue;
            input >> password;
        myUsers[username].password = password;
    }
    input.close();
}
void CentralServer::login(const str &username, const str &pw, Message* req, CentralServer* myCS)
{
    bool flag = false;
    if (myCS->is_registered(username, pw))
    {
        hamada curr_user;
        curr_user.active = 1;
        curr_user.socket_address = req->getSource();
	    curr_user.password = pw;
        myCS->myUsers[username] = curr_user;
        flag = true;
    }
    str m = (flag) ? "Successful" : "Incorrect Username or Password";
    char * c = new char[m.size()+1];
    strcpy(c, m.c_str());
    req->setAck();
    auto it = req->getDestination();
    Message *myMsg = new Message(Reply,m.size()+1,req->getrpc_Id(),0,0,req->getOperation(),c,it.first, it.second);
    myCS->sendMessage(myMsg);
}

void CentralServer::signup(const str &username,const  str &pw, Message* req,CentralServer* myCS)
{
    bool flag = true;
    if (myCS->user_found(username))
        flag = false;
    if (flag){
        myCS->myUsers[username].password = pw;
        myCS->output.open(myCS->filepath.c_str());
        if (myCS->output.fail()) std::cout << "eh?\n";
        for (auto it =myCS->myUsers.begin(); it != myCS->myUsers.end(); ++it)
            myCS->output << it->first << ' ' << it->second.password << '\n';
        myCS->output.close();
    }
    str m = (flag) ? "Successful": "Unsuccessful";

    char * c = new char[m.size()+1];
    strcpy(c, m.c_str());
    req->setAck();
    auto it = req->getDestination();
    Message *myMsg = new Message(Reply,m.size()+1,req->getrpc_Id(),0,0,req->getOperation(),c,it.first, it.second);
    myCS->sendMessage(myMsg); 

}

bool CentralServer::is_registered(const str &username, const str &pw)
{
    //check later for duplicate users not users and password
    if (myUsers.find(username) != myUsers.end())
        return myUsers[username].password == pw;
    return false;
}
bool CentralServer::user_found(const str &username)
{
    return myUsers.find(username) != myUsers.end();
}
void CentralServer::requestImages(str Usrname, Message* req, CentralServer* myCS)
{
    html_syntax head;
    str out = head.html_beg;
    std::ofstream xx("testing.txt");
    for (auto it = myCS->myUsers.begin(); it != myCS->myUsers.end(); ++it)
    {
        if (it->second.active && it->first != Usrname){
            hamada current = it->second;
            out += head.user_beg + head.header_beg + it->first + head.header_end + head.ip_beg 
                    + current.socket_address + head.ip_end 
                    + head.body_beg;
            for (int i = 0; i < current.images.size(); ++i)
            {
                out += head.list_beg;
                out += current.images[i];
                out += head.list_end;
            }
            out += head.body_end;
            out += head.user_end;
        }
    }
    out += head.html_end;
    xx << out << '\n';
    char * c = new char[out.size()+1];
    strcpy(c, out.c_str());
    req->setAck();
    auto it = req->getDestination();

    Message *myMsg = new Message(Reply,out.size()+1,req->getrpc_Id(),0,0,req->getOperation(),c,it.first, it.second);
    myCS->sendMessage(myMsg);

}

void CentralServer::uploadimage(const str &username, const str &imagename,  Message* req, CentralServer *myCS)
{
    myCS->myUsers[username].images.push_back(imagename);
    str m = "Successful";

    char * c = new char[m.size()+1];
    strcpy(c, m.c_str());
    req->setAck();
    auto it = req->getDestination();
    Message *myMsg = new Message(Reply,m.size()+1,req->getrpc_Id(),0,0,req->getOperation(),c,it.first, it.second);
    myCS->sendMessage(myMsg); 
}
void CentralServer::doOperation(Message *request)
{
    int operationID = request->getOperation();
    std::thread* receiverThread;
    str input = str((char*)request->getMessage());
    int found = input.find(',');
    str first = input.substr(0, found);
    str second, third;
    if (found != str::npos) //two argument
    {
        second = input.substr(found+1);
    }
    if (operationID == 0)
    {
        receiverThread = new std::thread(CentralServer::login, first, second ,request, this); //done
    }
    else if (operationID == 1) 
    {
        receiverThread = new std::thread(CentralServer::signup, first, second, request, this); //done
    }
    else if (operationID == 2) 
    {
        receiverThread = new std::thread(CentralServer::uploadimage, first, second, request,this); //done
    }
    else if (operationID == 3)
    {
        receiverThread = new std::thread(CentralServer::logout, first, request,this); //done
    }
    else if (operationID == 4) 
    {
        receiverThread = new std::thread(CentralServer::requestImages, first, request,this); //done
    }
}
void CentralServer::listen()
{
    Message* myMessage;
    while (1)
    {
        if (this->checkMessages(myMessage))
        {
            std::cout << "Message itself "<<(char*) myMessage->getMessage() << '\n';
            std::cout << "operationID " << myMessage->getOperation() << '\n';
           this->doOperation(myMessage); 
        }
    }
}
void CentralServer::logout(const str& username, Message* req, CentralServer *myCS)
{
   myCS->myUsers[username].active = 0;
   str m = "Successful";
   char * c = new char[m.size()+1];
   strcpy(c, m.c_str());
   req->setAck();
   auto it = req->getDestination();
   Message *myMsg = new Message(Reply,m.size()+1,req->getrpc_Id(),0,0,req->getOperation(),c,it.first, it.second);
   myCS->sendMessage(myMsg); 
}
CentralServer::~CentralServer()
{

}
