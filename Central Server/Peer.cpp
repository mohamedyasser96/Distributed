#include "Peer.h"

Peer::Peer(int port):UDPSocket(port){
    
    int RPC_ID = 0;

    this->ServerIP = "123.12.1.23";
    this->ServerPort = 8080;

    this->flag = true;
    handler = new std::thread(&Peer::MessageHandler,this);
}

void Peer::LogInServer(std::string userName, std::string Password){
    
    std::string msg = userName + "," + Password;

    char * c = new char[msg.size()+1];
    strcpy(c, msg.c_str());

    Message * m = new Message(Request,msg.size()+1, this->RPC_ID,0,0,LOGIN_OP,c,this->ServerIP,this->ServerPort);

    this->sendMessage(m);

}
void Peer::RegisterServer(std::string userName, std::string Password){
    
    std::string msg = userName + "," + Password;

    char * c = new char[msg.size()+1];
    strcpy(c, msg.c_str());

    Message * m = new Message(Request,msg.size()+1, this->RPC_ID,0,0,SIGNUP_OP,c,this->ServerIP,this->ServerPort);

    this->sendMessage(m);

}

void Peer::LogOutServer(std::string userName){
     
     std::string msg = userName;

    char * c = new char[msg.size()+1];
    strcpy(c, msg.c_str());

    Message * m = new Message(Request,msg.size()+1, this->RPC_ID,0,0,LOGOUT_OP,c,this->ServerIP,this->ServerPort);

    this->sendMessage(m);

}

void Peer::UploadImageServer(std::string userName, std::string imgName){

    std::string msg = userName + "," + imgName;

    char * c = new char[msg.size()+1];
    strcpy(c, msg.c_str());

    Message * m = new Message(Request,msg.size()+1, this->RPC_ID,0,0,UPIM_OP,c,this->ServerIP,this->ServerPort);

    this->sendMessage(m);


}

void Peer::RequestImagesServer(){

    std::string msg = "";

    char * c = new char[msg.size()+1];
    strcpy(c, msg.c_str());

    Message * m = new Message(Request,msg.size()+1, this->RPC_ID,0,0,RQIM_OP,c,this->ServerIP,this->ServerPort);

    this->sendMessage(m);


}

void Peer::RequestImagePeer(std::string imgName, std::string ip, int port){
    
    int size = imgName.size() + 1;
    char* c = new char[size];
    strcpy(c, imgName.c_str());
    Message* imageRequest = new Message(Request, size, this->RPC_ID++, 0,0, 5,  c, ip,  port);
    this->sendMessage(imageRequest);
}

void Peer::ChangeFlag(){
    flag = !flag;
}
void Peer::myHandler(Message *myMSG)
{
    int operationID = myMSG->getOperation();
    std::ofstream x("output.txt");
    x << "eh!\n";
    x.close();
    if (operationID == 0) //handle login
    {

    }
    else if (operationID == 1) //handle register
    {

    }
    else if (operationID == 2) //handle upload
    {
        
    }
    else if (operationID == 3) //handle logout
    {
        
    }
    else if (operationID == 4) //handle request images
    {
        
    }
    else if (operationID == 5) //handle image transfering
    {
        MessageType type = myMSG->getMessageType();
        if (type == Request) // A peer is requesting an image from me!
        {

            std::string hide_image = std::string((char*)myMSG->getMessage());
            Message* responseMessage = new Message();
            myMSG->setAck();
            auto it = myMSG->getDestination();
            std::string ip = it.first; 
            int port = it.second;
            sendImage(responseMessage, hide_image, ip, port);
            this->sendMessage(responseMessage);

        }
        else if (type == Reply) // A peer is sending an image to me!
        {
            saveImage(myMSG, "saved.jpg");
        }
    }
}

std::string Peer::retrieveImage(const std::string &path){

    std::ifstream input;
    input.open(path, std::ios_base::binary);

    input.seekg(0, std::ios::end);
    long long size = input.tellg();
    input.seekg(0, std::ios::beg);

    char * buffer = new char[size];

    input.read(buffer, size);
    input.close();

    std::string encoded = base64_encode(reinterpret_cast<const unsigned char*>(reinterpret_cast<const unsigned char*> (buffer)), size);
    
    delete [] buffer;
    return encoded; 

}

void Peer::MessageHandler(Peer * p){

    std::ofstream out("bye.txt");
    out << "Started thread: " << std::endl;
    out.flush(); 
    Message * m;
    while(true){  
            if(p->checkMessages(m))
            {
                //out << std::string((char*)m->getMessage()) << std::endl;
                p->myHandler(m);
            }
    }
}


void Peer::sendImage(Message*& image, std::string path, std::string ip , int port){

    std::ifstream input;
    input.open(path, std::ios_base::binary);

    input.seekg(0, std::ios::end);
    long long size = input.tellg();
    input.seekg(0, std::ios::beg);

    char * buffer = new char[size];

    input.read(buffer, size);
    input.close();

    std::string encoded = base64_encode(reinterpret_cast<const unsigned char*>(reinterpret_cast<const unsigned char*> (buffer)), size);
    
    delete buffer;
    buffer = new char[encoded.size()+1];
    strcpy(buffer, encoded.c_str());

    image = new Message(Reply, size, this->RPC_ID++, 0,0,5,  buffer, ip,  port);
}

void Peer::saveImage(Message* image, std::string path){

    std::string s = std::string((char*)image->getMessage());
    s = base64_decode(s);
    std::ofstream out(path.c_str());
    out << s;
    out.close();
}