#ifndef _SEGSTREAM_WITHOUT_UPLOAD_
#define _SEGSTREAM_WITHOUT_UPLOAD_

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include <sstream>
#include <string.h>
#include <limits.h>
#include <stdarg.h>  
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <sys/types.h>
#include "Config.h"

using namespace std;

string camerainfo;
string streaminfo;
string bitrate = "0";
string imageresolution = "";
string framerate = "0";


//判断是否为目录
bool is_dir(const char *path)
{
    struct stat statbuf;
    if(lstat(path, &statbuf) ==0)//lstat返回文件的信息，文件信息存放在stat结构中
        return S_ISDIR(statbuf.st_mode) != 0;//S_ISDIR宏，判断文件类型是否为目录
    return false;
}


// int 转 string
string ItoS(int n)
{
	stringstream ss;
	string str;
	ss<<n;
	ss>>str;
	return str;
}


//获取文件上一次修改的时间距当前时间的秒数
int lastOperateSeconds(string filePath)
{
	FILE * fp;
	int fd;
	struct stat buf;
	fp=fopen(filePath.c_str(),"r"); //C.zip in current directory, I use it as a test
	if(fp==NULL) //如果失败了
	{
		printf("错误！");
		return 31;
	}
	fd=fileno(fp);
	fstat(fd, &buf);
	int size = buf.st_size; //get file size (byte)
	long modify_time=buf.st_mtime; //latest modification time (seconds passed from 01/01/00:00:00 1970 UTC)
	time_t timep;
	long d = time(&timep);;
	int gap = difftime(d, modify_time);
	fclose(fp);
	return gap;
}


//判断指定文件是否存在
int JudgeFile(string dir)
{
	if(access(dir.c_str(), F_OK) == 0)
		return 1;
	else
		return -1;
}


//判断指定目录是否存在
int JudgeDir(string dir)
{
	struct stat fileStat;
	if((stat(dir.c_str(), &fileStat) == 0) && S_ISDIR(fileStat.st_mode))
		return 0;
	else
		return -1;
}


//创建目录
int CreateDir(const char *sPathName)  
{  
	char DirName[256];  
	strcpy(DirName, sPathName);  
	int i, len = strlen(DirName);  
	if(DirName[len-1]!='/')  
	strcat(DirName, "/");  

	len = strlen(DirName);  

	for(i=1; i<len; i++)  
	{  
		if(DirName[i] == '/')  
		{  
			DirName[i] = 0;  
			if( access(DirName, NULL) != 0 )  
			{  
				if( mkdir(DirName, 0777) == -1 )  
				{   
					perror("mkdir   error");   
					return -1;   
				}  
			}  
			DirName[i] = '/';  
		}  
	}  

	return 0;  
} 

void killprogress(string killgrogressname)
{
	string killCommandStr = "ps -ef|grep \"" + killgrogressname + "\" | grep -v grep | cut -c 9-15|xargs kill -9";
	cout << "killCommandStr=" << killCommandStr << endl;
	system(killCommandStr.c_str());
}


//获取文件大小
unsigned long get_file_size(const char *path)  
{  
    unsigned long filesize = 0;      
    struct stat statbuff;  
    if(stat(path, &statbuff) < 0){  
        return filesize;  
    }else{  
        filesize = statbuff.st_size;  
    }  
    return filesize;  
}  


void recordprogress(string ffmpegpath, int streamduration, string streamfilename, string sourcepath)
{
	string recordCommandStr;
	string tempStr = "";
	if(imageresolution != "")
		tempStr += " -s " + imageresolution + " ";
	if(bitrate != "0")
		tempStr += " -vb " + bitrate + "k -ab 0 ";
	if(framerate != "0")
		tempStr += " -r " + framerate + " ";

	if((bitrate == "0") && (framerate == "0") && (imageresolution == ""))
		tempStr = " -acodec copy -vcodec copy ";

	recordCommandStr = ffmpegpath + " -y -i " + sourcepath + tempStr + " -t " + ItoS(streamduration) + " -f flv " + streamfilename + " &";

	cout << "recordCommandStr=" << recordCommandStr << endl;
	system(recordCommandStr.c_str());

}



int readFileList(char *basePath, string Hour)
{
	string str=Hour+"-";
	int result=0;
        DIR *dp;
        struct dirent *dirp;
        char *dirname=basePath;
        
        //store file names
        vector<string> file_names;

        if((dp=opendir(dirname))==NULL){
                perror("opendir error");
                exit(1);
        }

        while((dirp=readdir(dp))!=NULL){
                if((strcmp(dirp->d_name,".")==0)||(strcmp(dirp->d_name,"..")==0))
                        continue;
		file_names.push_back(dirp->d_name);
		string str1 = dirp->d_name;
		string::size_type pos = str1.find(str);
		if(pos != string::npos)
		{
			str1.erase(pos , str.length());
			cout << str1 << endl;
			result++;
		}
        }

	closedir(dp);
	return result;
}


string getvidoeinfo(string str)
{
    size_t bpos = 0;
    size_t spos = 0;
    size_t fpos = 0;
    string findB = "b=";//标记码率
    string findS = "s=";//标记分辨率
    string findF = "f=";//标记帧率
	
    bpos = str.find(findB);
    spos = str.find(findS);
    fpos = str.find(findF);
    
    string tempstr = str.substr(2);
    if(bpos != string::npos)
	bitrate = tempstr;
    else if(spos != string::npos)
	imageresolution = tempstr;
    else if(fpos != string::npos)
	framerate = tempstr;

    return "";
}



int main(int argc, char **argv)
{
//    const char ConfigFile[]= "/home/centos/work/temp/config.txt";   
	string ConfigFile= argv[1];   
	Config configSettings(ConfigFile);

	camerainfo=configSettings.Read("camerainfo", camerainfo);  
	streaminfo=configSettings.Read("streaminfo", streaminfo);  
	string applicationpath=configSettings.Read("applicationpath", applicationpath);  
	string ffmpegpath=configSettings.Read("ffmpegpath", ffmpegpath);  
	bitrate=configSettings.Read("bitrate", bitrate);  
	imageresolution=configSettings.Read("imageresolution", imageresolution);  
	framerate=configSettings.Read("framerate", framerate);  
	cout << "camerainfo=" << camerainfo << endl;
	cout << "streaminfo=" << streaminfo << endl;
	cout << "applicationpath=" << applicationpath << endl;
	cout << "ffmpegpath=" << ffmpegpath << endl;
	cout << "bitrate=" << bitrate << endl;
	cout << "framerate=" << framerate << endl;
	cout << "imageresolution=" << imageresolution << endl;

	string basepath = applicationpath + "/streamfile";
	string Year, Month, Day, Hour, Minute, Second;

	 time_t tt = time(NULL);//这句返回的只是一个时间戳
	 tm* t= localtime(&tt);
	 
	 char ye[4];
	 char mo[2];
	 char da[2];
	 char h[2];
	 char m[2];
	 char s[2];
	 printf("aaaa %d-%02d-%02d %02d:%02d:%02d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	 sprintf(ye, "%04d", (t->tm_year + 1900));
	 sprintf(mo, "%02d", (t->tm_mon + 1));
	 sprintf(da, "%02d", t->tm_mday);
	 sprintf(h, "%02d", t->tm_hour);
	 sprintf(m, "%02d", t->tm_min);
	 sprintf(s, "%02d", t->tm_sec);
	 printf("%d-%02d-%02d %02d:%02d:%02d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	 
	 Year = ye;
	 Month = mo;
	 Day = da;
	 Hour = h;
	 Minute = m;
	 Second = s;

	int streamduration = 0;
	int minutes = atoi(Minute.c_str());	 
	if(minutes > 0)
		streamduration = (60 - minutes)*60;
	else
		streamduration = 0;

	int nowhour = atoi(Hour.c_str());
	while(nowhour>=0 && nowhour<24)
	{
		string ossbasepath = camerainfo + "/" + Year + "/" + Month + "/" + Day + "/" + Hour + "/";
		string osspath = "";

		int lastoperatefiletime;
		int streamfilenum = 0;

		if(streamduration < 60)
			streamduration=3600;

		string streambasefilepath = applicationpath + "/streamfile/" + Year + "-" + Month + "-" + Day + "/" + camerainfo + "/";

		int ossfilenum = 0;
		unsigned long streamfilesize=0;
		float filesize_k;
		float filesize_m;

		if(is_dir(streambasefilepath.c_str()))
			streamfilenum = readFileList((char *)streambasefilepath.data(), Hour);
		else
			CreateDir(streambasefilepath.c_str());
		cout << " streamfilenum = " << streamfilenum << endl;

		streambasefilepath = applicationpath + "/streamfile/" + Year + "-" + Month + "-" + Day + "/" + camerainfo + "/" + Hour + "-";

		string streamfilename = streambasefilepath + ItoS(streamfilenum) + ".flv";
		recordprogress(ffmpegpath,streamduration, streamfilename, streaminfo);
		sleep(30);

		streamfilesize = get_file_size(streamfilename.c_str());
		cout << endl << "streamfilesize=" << streamfilesize << endl;
		if(bitrate != "0"){
			int tempduration = streamfilesize/1024/(atoi(bitrate.c_str())/8);
			streamduration = streamduration - tempduration;
		}
		else
			streamduration = streamduration - 30;

		lastoperatefiletime = lastOperateSeconds(streamfilename);

		while(streamduration > 0)
		{
		    while(lastoperatefiletime > 30){
			cout << "streamfilename=" << streamfilename << endl;
			killprogress( streamfilename );

			streamfilesize = get_file_size(streamfilename.c_str());
			filesize_k = streamfilesize/1024.0;
			filesize_m = filesize_k/1024.0;
			if(filesize_m > 8.0)
				osspath = ossbasepath + ItoS(ossfilenum) + ".flv";
			else{
				if(ossfilenum > 0)
					ossfilenum--;
			}

			if(streamduration > 0)
			{
				unsigned long lastfilesize = get_file_size(streamfilename.c_str());
				cout << "lastfilesize=" << lastfilesize << endl;
				int lastfileoperateseconds = lastOperateSeconds(streamfilename.c_str());
				cout << "lastfileoperateseconds=" << lastfileoperateseconds << endl;
				if(JudgeFile(streamfilename.c_str()) > 0){
					streamfilenum++;
					streamfilename = streambasefilepath + ItoS(streamfilenum) + ".flv";
					recordprogress(ffmpegpath, streamduration, streamfilename, streaminfo);
					ossfilenum++;
				}
				else
					recordprogress(ffmpegpath, streamduration, streamfilename, streaminfo);
			}
			else
				break;
			sleep(30);
			streamduration = streamduration - 30;
			lastoperatefiletime = lastOperateSeconds(streamfilename);
		    }
		    sleep(30);
		    streamduration = streamduration - 30;
		    lastoperatefiletime = lastOperateSeconds(streamfilename);
		    cout << "lastoperatefiletime=" << lastoperatefiletime << ", streamduration=" << streamduration << endl;
		}

		cout << "streamfilename=" << streamfilename << endl;
		killprogress( streamfilename );

		streamfilesize = get_file_size(streamfilename.c_str());
		filesize_k = streamfilesize/1024.0;
		filesize_m = filesize_k/1024.0;
		if(filesize_m > 8.0)
			osspath = ossbasepath + ItoS(ossfilenum) + ".flv";
		else{
			if(ossfilenum > 0)
				ossfilenum--;
		}

		tt = time(NULL);//这句返回的只是一个时间戳
		t= localtime(&tt);
		sprintf(ye, "%04d", (t->tm_year + 1900));
		sprintf(mo, "%02d", (t->tm_mon + 1));
		sprintf(da, "%02d", t->tm_mday);
		sprintf(h, "%02d", t->tm_hour);
		sprintf(m, "%02d", t->tm_min);
		sprintf(s, "%02d", t->tm_sec);
		printf("%d-%02d-%02d %02d:%02d:%02d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
		 
		Year = ye;
		Month = mo;
		Day = da;
		Hour = h;
		Minute = m;
		Second = s;

		cout << "streambasefilepath=" << streambasefilepath << endl;
		cout << "nowhour=" << nowhour << ", Hour=" << Hour << endl;

		nowhour = atoi(Hour.c_str());
	}


	return 0;
}
#endif