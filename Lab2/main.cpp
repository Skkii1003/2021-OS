#include<iostream>
#include<string>
#include<vector>
#include<sstream>
#include<cstring>
#include<stdio.h>
#include<string.h>

using namespace std;

extern "C" {
	void my_print(const char *, const int);
}

//void my_print(const char* str, const int len) {
//    for (int i = 0; i < len; i++) {
//        cout << str[i];
//    }
//}

typedef unsigned char u1;
typedef unsigned short u2;
typedef unsigned int u4;

char err_name[] = "You can set name for only one time.\n";
char err_input[] = "Invalid input.\n";
char err_ins[] = "Invalid ins.\n";
char err_fseek[] = "Failed in fseek.\n";
char err_fread[] = "Failed in fread.\n";
char err_find[] = "Can't find the dir or file.\n";
char err_file[] = "Can't read this file.\n";

string hint = "> ";

#pragma pack (1)
struct BPB {
    u2 BPB_BytesPerSec; 
    u1 BPB_SecPerClus; 
    u2 BPB_RsvdSecCnt; 
    u1 BPB_NumFATs; 
    u2 BPB_RootEntCnt;
    u2 BPB_TotSec16;
    u1 BPB_Media; 
    u2 BPB_FATSz16; 
    u2 BPB_SecPerTrk; 
    u2 BPB_NumHeads; 
    u4 BPB_HiddSec; 
    u4 BPB_TotSec32; 
};

struct RootEnt {
    char DIR_Name[11]; 
    u1 DIR_Attr; 
    char RER[10]; 
    u2 DIR_WrtTime; 
    u2 DIR_WrtDate; 
    u2 DIR_FstClus; 
    u4 DIR_FileSize; 
};
#pragma pack ()

class Node {
public:
    string fileName;
    string filePath;
    u4 fileSize;
    vector<Node*> next;
    bool isDir = true;
    bool isDot = false;
    int numSubDir = 0;
    int numSubFile = 0;
    vector<char> content;
};
//read BPB and rootEnt, create link
void ReadFat12(FILE* fat12, BPB* bpbPtr, RootEnt* rootEntPtr, Node* root);
//read subdir
void ReadSubDir(FILE* fat12, BPB* bpbPtr, u2 FstClus, Node* root);
//read file
void ReadContent(FILE* fat12, BPB* bpbPtr, u2 FstClus, Node* root);
//print ls
void printLs(Node* root, string name, bool setL);
//find the node in the link
Node* findNode(Node* root, string path);
//print cat
void printCat(Node *root, string name);

int main() {
    FILE* fat12;
    fat12 = fopen("a.img", "rb"); //read as binary

    BPB bpb;
    BPB* bpbPtr = &bpb;
    RootEnt rootEnt;
    RootEnt* rootEntPtr = &rootEnt;

    Node* root = new Node();
    root->fileName = "";
    root->filePath = "/";

    //read fat12 and create link
    ReadFat12(fat12, bpbPtr, rootEntPtr, root);

    //input instruction
    string in;
    my_print(hint.c_str(), strlen(hint.c_str()));
    while (getline(cin, in)) {
            stringstream ss(in);
            string ins;
            string part;
            string name = "/";
            bool err = false;
            ss >> ins;

            if (ins == "ls") {
                bool setName = false;
                bool setL = false;
                while (ss >> part) {
                    if (part[0] == '/') {
                        if (!setName) {
                            name = part + name;
                            setName = true;
                        }
                        else {
                            my_print(err_name, strlen(err_name));
                            err = true;
                            break;
                        }
                        continue;
                    }
                    else if (part[0] == '-') {
                        int size = part.length();
                        if (size == 1) {
                            my_print(err_input, strlen(err_input));
                            err = true;
                            break;
                        }

                        for (int i = 1; i < size; i++) {
                            if (part[i] == 'l')
                                setL = true;
                            else {
                                my_print(err_input, strlen(err_input));
                                err = true;
                                break;
                            }
                        }
                    }
                    else {
                        my_print(err_input, strlen(err_input));
                        err = true;
                        break;
                    }
                }

                if (err)
                    continue;

                printLs(root, name, setL);
            }
            else if (ins == "cat") {
                ss >> part;
                if (part == "" || part[part.length()-1] =='/')
                    my_print(err_input, strlen(err_input));

                printCat(root, part);
            }
            else if (ins == "exit") {
                break;
            }
            else {
                my_print(err_ins, strlen(err_ins));
            }
            my_print(hint.c_str(), strlen(hint.c_str()));
        }
}

void ReadFat12(FILE* fat12, BPB* bpbPtr, RootEnt* rootEntPtr, Node* root) {
    //read BPB
    int ret = 0;

    ret = fseek(fat12, 11, SEEK_SET);
    if (ret == -1)
        my_print(err_fseek, strlen(err_fseek));

    ret = fread(bpbPtr, 1, 25, fat12);
    if (ret != 25)
        my_print(err_fread, strlen(err_fread));


    //read RootEnt
    int offset = (bpbPtr->BPB_RsvdSecCnt + bpbPtr->BPB_NumFATs * bpbPtr->BPB_FATSz16) * bpbPtr->BPB_BytesPerSec;

    for (int i = 0; i < bpbPtr->BPB_RootEntCnt; i++) {

        ret = fseek(fat12, offset, SEEK_SET);
        if (ret == -1)
            my_print(err_fseek, strlen(err_fseek));

        ret = fread(rootEntPtr, 1, 32, fat12);
        if (ret != 32)
            my_print(err_fread, strlen(err_fread));

        offset += 32;

        //not valid row
        bool valid = true;
        for (int j = 0; j < 11; j++) {
            if (rootEntPtr->DIR_Name[j] < '0' || rootEntPtr->DIR_Name[j]>'9') {
                if (rootEntPtr->DIR_Name[j] < 'A' || rootEntPtr->DIR_Name[j] > 'Z') {
                    if (rootEntPtr->DIR_Name[j] != ' ') {
                        valid = false;
                        break;
                    }
                }
            }
        }
        if (!valid)
            continue;

        char name[12];
        int index = 0;
        if ((rootEntPtr->DIR_Attr & 16) == 0x10) {
            //this is a dir
            for (int j = 0; j < 11; j++) {
                if (rootEntPtr->DIR_Name[j] != ' ') {
                    name[index] = rootEntPtr->DIR_Name[j];
                    index++;
                }
                else {
                    name[index] = '\0';
                    break;
                }
            }

            //create dir node in root
            Node* son = new Node();
            root->next.push_back(son);
            son->fileName = name;
            son->filePath = root->filePath + name + "/";
            root->numSubDir++;
            //create dot node in son
            Node* dot = new Node();
            dot->fileName = ".";
            dot->isDot = true;
            son->next.push_back(dot);
            dot = new Node();
            dot->fileName = "..";
            dot->isDot = true;
            son->next.push_back(dot);
            //read sub dir
            ReadSubDir(fat12, bpbPtr, rootEntPtr->DIR_FstClus, son);
        }
        else {
            //this is a file
            index = 0;
            for (int j = 0; j < 11; j++) {
                if (rootEntPtr->DIR_Name[j] != ' ') {
                    name[index] = rootEntPtr->DIR_Name[j];
                    index++;
                }
                else {
                    name[index] = '.';
                    index++;
                    j++;
                    while (rootEntPtr->DIR_Name[j] == ' ')
                        j++;
                    j--;
                }
            }
            name[index] = '\0';

            //create file node in root
            Node* son = new Node();
            root->next.push_back(son);
            root->numSubFile++;
            son->fileName = name;
            son->filePath = root->filePath + name;
            son->isDir = false;
            son->fileSize = rootEntPtr->DIR_FileSize;
            ReadContent(fat12, bpbPtr, rootEntPtr->DIR_FstClus, son);
        }
    }
}

void ReadSubDir(FILE* fat12, BPB* bpbPtr, u2 FstClus, Node* root) {
    int base = bpbPtr->BPB_RsvdSecCnt + bpbPtr->BPB_NumFATs * bpbPtr->BPB_FATSz16 + (bpbPtr->BPB_RootEntCnt * 32 + bpbPtr->BPB_BytesPerSec - 1) / bpbPtr->BPB_BytesPerSec;
    int offset = (base + FstClus - 2) * bpbPtr->BPB_BytesPerSec;

    RootEnt sonEnt;
    RootEnt* sonEntPtr = &sonEnt;

    int ret = 0;
    int end = bpbPtr->BPB_SecPerClus * bpbPtr->BPB_BytesPerSec;
    for (int i = 0; i < end; i += 32) {

        ret = fseek(fat12, offset, SEEK_SET);
        if (ret == -1)
            my_print(err_fseek, strlen(err_fseek));

        ret = fread(sonEntPtr, 1, 32, fat12);
        if (ret != 32)
            my_print(err_fread, strlen(err_fread));

        offset += 32;

        //not valid row
        bool valid = true;
        for (int j = 0; j < 11; j++) {
            if (sonEntPtr->DIR_Name[j] < '0' || sonEntPtr->DIR_Name[j]>'9') {
                if (sonEntPtr->DIR_Name[j] < 'A' || sonEntPtr->DIR_Name[j] > 'Z') {
                    if (sonEntPtr->DIR_Name[j] != ' ') {
                        valid = false;
                        break;
                    }
                }
            }
        }
        if (!valid)
            continue;

        char name[12];
        int index = 0;
        if ((sonEntPtr->DIR_Attr & 16) == 0x10) {
            //this is a dir
            for (int j = 0; j < 11; j++) {
                if (sonEntPtr->DIR_Name[j] != ' ') {
                    name[index] = sonEntPtr->DIR_Name[j];
                    index++;
                }
                else {
                    name[index] = '\0';
                    break;
                }
            }

            //create dir node in root
            Node* son = new Node();
            root->next.push_back(son);
            son->fileName = name;
            son->filePath = root->filePath + name + "/";
            root->numSubDir++;
            //create dot node in son
            Node* dot = new Node();
            dot->fileName = ".";
            dot->isDot = true;
            son->next.push_back(dot);
            dot = new Node();
            dot->fileName = "..";
            dot->isDot = true;
            son->next.push_back(dot);
            //read sub dir
            ReadSubDir(fat12, bpbPtr, sonEntPtr->DIR_FstClus, son);
        }
        else {
            //this is a file
            index = 0;
            for (int j = 0; j < 11; j++) {
                if (sonEntPtr->DIR_Name[j] != ' ') {
                    name[index] = sonEntPtr->DIR_Name[j];
                    index++;
                }
                else {
                    name[index] = '.';
                    index++;
                    j++;
                    while (sonEntPtr->DIR_Name[j] == ' ')
                        j++;
                    j--;
                }
            }
            name[index] = '\0';

            //create file node in root
            Node* son = new Node();
            root->next.push_back(son);
            root->numSubFile++;
            son->fileName = name;
            son->filePath = root->filePath + name;
            son->isDir = false;
            son->fileSize = sonEntPtr->DIR_FileSize;
            ReadContent(fat12, bpbPtr, sonEntPtr->DIR_FstClus, son);
        }
    }
}

void ReadContent(FILE* fat12, BPB* bpbPtr, u2 FstClus, Node* root) {
    int base = bpbPtr->BPB_RsvdSecCnt + bpbPtr->BPB_NumFATs * bpbPtr->BPB_FATSz16 + (bpbPtr->BPB_RootEntCnt * 32 + bpbPtr->BPB_BytesPerSec - 1) / bpbPtr->BPB_BytesPerSec;
    int offset = (base + FstClus - 2) * bpbPtr->BPB_BytesPerSec;

    int ret = 0;
    char content[1000];
    int size = bpbPtr->BPB_BytesPerSec * bpbPtr->BPB_SecPerClus;

    ret = fseek(fat12, offset, SEEK_SET);
    if (ret == -1)
        my_print(err_fseek, strlen(err_fseek));

    ret = fread(content, 1, size, fat12);
    if (ret != size)
        my_print(err_fread, strlen(err_fread));

    for (int i = 0; i < size; i++) {
        if (content[i] == EOF || content[i] == '\0') {
            break;
        }
        else {
            root->content.push_back(content[i]);
        }
    }
}

void printLs(Node* root, string path, bool setL) {
    Node* father = findNode(root, path);
    Node* son;

    if (father == nullptr) {
        my_print(err_find, strlen(err_find));
        return;
    }

    string out;
    if (!setL) {
        out = father->filePath + ":\n";
        my_print(out.c_str(), strlen(out.c_str()));
        for (u4 i = 0; i < father->next.size(); i++) {
            son = father->next[i];
            if (son->isDir) {
                out = "\033[31m" + son->fileName + "\033[0m";
                my_print(out.c_str(), strlen(out.c_str()));
            }
            else {
                out = son->fileName;
                my_print(out.c_str(), strlen(out.c_str()));
            }

            if (i < father->next.size() - 1) {
                out = "  ";
                my_print(out.c_str(), strlen(out.c_str()));
            }
            else {
                out = "\n";
                my_print(out.c_str(), strlen(out.c_str()));
            }
        }
        out = "\n";
        my_print(out.c_str(), strlen(out.c_str()));

        for (u4 i = 0; i < father->next.size(); i++) {
            son = father->next[i];
            if (son->isDir && !son->isDot)
                printLs(son, son->filePath, setL);
        }
    }
    else {
        out = father->filePath + " " + to_string(father->numSubDir) + " " + to_string(father->numSubFile) + ":\n";
        my_print(out.c_str(), strlen(out.c_str()));
        for (u4 i = 0; i < father->next.size(); i++) {
            son = father->next[i];
            if (son->isDot) {
                out = "\033[31m" + son->fileName + "\033[0m" + "\n";
                my_print(out.c_str(), strlen(out.c_str()));
            }
            else {
                if (son->isDir) {
                    out = "\033[31m" + son->fileName + "\033[0m" + "  " + to_string(son->numSubDir) + " " + to_string(son->numSubFile) + "\n";
                }
                else
                    out = son->fileName + "  " + to_string(son->fileSize) + "\n";
                my_print(out.c_str(), strlen(out.c_str()));
            }
        }

        out = "\n";
        my_print(out.c_str(), strlen(out.c_str()));

        for (u4 i = 0; i < father->next.size(); i++) {
            son = father->next[i];
            if (son->isDir && !son->isDot)
                printLs(son, son->filePath, setL);
        }
    }
}

Node* findNode(Node* root, string path) {
    if (root->filePath == path)
        return root;

    int size = root->next.size();
    Node* ret;
    for (int i = 0; i < size; i++) {
        if (root->next[i]->isDir) {
            if (root->next[i]->filePath == path)
                return root->next[i];
            else {
                ret = findNode(root->next[i], path);
                if (ret == nullptr)
                    continue;
                else
                    return ret;
            }
        }
        else {
            if (root->next[i]->filePath == path) {
                return root->next[i];
            }
            continue;
        }
    }
    return nullptr;
}

void printCat(Node* root, string name) {
    string out = "";

    if (name[0] != '/')
        name = "/" + name;

    Node* file = findNode(root, name);
    if (file == nullptr) {
        my_print(err_find, strlen(err_find));
    }
    else {
        out.insert(out.begin(), file->content.begin(), file->content.end());
        out = out + "\n";
        my_print(out.c_str(), strlen(out.c_str()));
    }
}