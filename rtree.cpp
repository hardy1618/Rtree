#define loop(i,k,n) for (int i = k; i < n; i++) 
#include<bits/stdc++.h>
#include<iostream>
#include "file_manager.h"
#include "errors.h"
#include<cstring>
#include <fstream>
#include "constants.h"

using namespace std;



vector<string> split(string a){
	vector<string> p;
	string temp="";
	for(auto i: a){
		if(i!=' '){
			temp= temp+ i;
		}
		else {p.push_back(temp);temp="";}
	}
	p.push_back(temp);
	return p;
}

vector<int> levels;
vector<PageHandler> levelpages;
vector<FileHandler> levelfiles;
vector<char *> leveldata;
int d,maxcap,nodesize,pagecap,page_size,intsize,buffersize;
FileManager fm;

// Optimization idea -- Dont place node everywhere. Directly memcpy 

char sz[10000];
void fun(int i){
	// Agar maxcap entries bhaari hai level[i] me
	// Since phela maxcap hamesha pagecap se chota rahega so
	if(levels[i]==1){
		cout<<"1"<<endl;
		if(levelfiles.size()<buffersize){
		    sprintf(sz, "%d.txt", i+1);
			levelfiles.push_back(fm.CreateFile(sz));
			levels.push_back(1);
			levelpages.push_back(levelfiles.back().NewPage());
			leveldata.push_back(levelpages.back().GetData());
			int node[nodesize],node1[nodesize];
			loop(j,0,nodesize) node[j]=0;
			memcpy(&node1,&leveldata[i][0],nodesize*intsize);
			loop(j,0,d){
				node[2+j]=node1[2+j];
				node[2+d+j]= node1[2+d+j];
				node[2*d+ 2]=levels[j];
				node[2*d+ 3+j]= node1[2+j];
				node[3*d + 3+ j]= node1[d +2+j];
			}
			memcpy(&leveldata.back()[0], &node, nodesize*intsize);
		}

	}
	else if(levels[i]%maxcap !=1){
		cout<<"2"<<endl;
		int childkanodenumber=(levels[i]-1)%pagecap;
		int papakanodenumber= (levels[i+1]-1)%pagecap;
		int papamechildnumber= (levels[i]-1)%maxcap;
		int node[nodesize],childnode[nodesize];
		memcpy(&node,&leveldata[i+1][papakanodenumber*nodesize],nodesize*intsize);
		memcpy(&childnode,&leveldata[i][childkanodenumber*nodesize],nodesize*intsize);
		loop(j,0,d){
			node[2+2*d+ papamechildnumber*(2*d+1)]=levels[i]-1;
			node[3+2*d +j+ papamechildnumber*(2*d+1)]=childnode[2+j];
			node[3+3*d +j+ papamechildnumber*(2*d+1)]=childnode[2+d+j];
			node[2+j]= min(node[2+j],childnode[2+j]);
			node[2+d+j]=max(node[2+d+j],childnode[2+d+j]);
		}

		memcpy(&leveldata[i+1][papakanodenumber*nodesize],&node,nodesize*intsize);
		// loop(j,0,nodesize) cout<<j<<" = "<<node[j]<<endl;

	}
	else{
		cout<<"3"<<endl;
		fun(i+1);
		cout<<"YEAH"<<endl;
		int childkanodenumber=(levels[i]-1)%pagecap;
		int papakanodenumber= (levels[i+1]-1)%pagecap;
		int papamechildnumber= 0;
		int node[nodesize],childnode[nodesize];
		memcpy(&node,&leveldata[i+1][papakanodenumber*nodesize],nodesize*intsize);
		memcpy(&childnode,&leveldata[i][childkanodenumber*nodesize],nodesize*intsize);
		loop(j,0,d){
			node[2+2*d+ papamechildnumber*(2*d+1)]=levels[i]-1;
			node[3+2*d +j+ papamechildnumber*(2*d+1)]=childnode[2+j];
			node[3+3*d +j+ papamechildnumber*(2*d+1)]=childnode[2+d+j];
			node[2+j]= min(node[2+j],childnode[2+j]);
			node[2+d+j]=max(node[2+d+j],childnode[2+d+j]);
		}

		memcpy(&leveldata[i+1][papakanodenumber*nodesize],&node,nodesize*intsize);
		loop(j,0,nodesize) cout<<j<<" = "<<node[j]<<endl;

	}
	if(levels[i]%pagecap == 0){
		int pagenumber = levelpages[i].GetPageNum();
		levelfiles[i].MarkDirty(pagenumber);
		levelfiles[i].UnpinPage(pagenumber);
		levelpages[i]=levelfiles[i].NewPage();
		leveldata[i]=levelpages[i].GetData();
	}
	levels[i]++;

}






void bulkload(string location ,int N){
	levelfiles.push_back(fm.CreateFile("0.txt"));
	levels.push_back(1);
	levelpages.push_back(levelfiles.back().NewPage());
	leveldata.push_back(levelpages.back().GetData());
	FileHandler fh= fm.OpenFile("Testcases/TC_1/sortedData_2_10_100.txt");
	PageHandler ph = fh.FirstPage();
	char *data = ph.GetData ();
	int countread=0,ids;
	int node[nodesize];
	loop(j,0,nodesize) node[j]=0;
	loop(i,0,d){
		node[d+2+i]=INT_MIN;
		node[2+i]= INT_MAX;
	}
	loop(i,0,N){
		if((i!=0 && i%maxcap==0) || (i==N-1) ){
			// node ko write page me likhna hai
			// cout<<"yeah"<<endl;
			memcpy(&leveldata[0][(levels[0]%pagecap)*nodesize], &node, intsize*nodesize);

			loop(j,0,nodesize) node[j]=0;
			loop(j,0,d){
				node[d+2+j]=INT_MIN;
				node[2+j]= INT_MAX;
			}
			fun(0);
		}
		node[0]=levels[0];
		int num;
		loop(j,0,d){
			if(countread==page_size){   //if reader page is full and needs new page
				countread=0;
				ph=fh.NextPage(ph.GetPageNum());
				data=ph.GetData();
			}
			memcpy (&num, &data[countread], intsize);
			node[d+2+j]=max(node[d+2+j],num);
			node[2+j]=min(node[2+j],num);
			node[2*d+3 + (i%maxcap) *(2*d+1)+j]=num;
			countread+=intsize;
		}

	}
	fm.DestroyFile("0.txt");
	fm.DestroyFile("1.txt");
	fm.DestroyFile("2.txt");

	// int temp[nodesize];
	// memcpy(&temp[0],&leveldata[0][nodesize],nodesize*intsize);
	// loop(i,0,nodesize) cout<<i<<" = "<<temp[i]<<endl;
}











int main( int argc, char *argv[]) {
	fstream newfile,file;	
	// loop(i,1,argc) cout<<argv[i]<<endl;
	char* p;
	maxcap = strtol(argv[2], &p, 10);
	page_size= PAGE_CONTENT_SIZE;
	buffersize= BUFFER_SIZE;
	intsize= sizeof(int);
	file.open(argv[1],ios::in); 
	string qw;
	getline(file,qw);
	getline(file,qw);
	d=split(qw).size()-1;
	nodesize= 2+ 2*d + maxcap*(1+2*d);
	int ae[nodesize];
	// cout<<nodesize<<" "<<sizeof(ae)<<endl;
	pagecap= int(page_size/sizeof(ae));
	// cout<<pagecap<<endl;
	newfile.open(argv[1],ios::in); 
	if (newfile.is_open()){ 
		string tp;	
		vector<string> temp;
		while(getline(newfile, tp)){
			temp= split(tp);
			if(temp[0]=="BULKLOAD"){
				bulkload(temp[1],stoi(temp[2]));
				cout<<"tisdfsd"<<endl;
				// test();
			}
			else if(temp[0]=="INSERT"){
				// insert(int(temp[1]),int(temp[2]));
			}
			else{
				// query(int(temp[1]),int(temp[2]));
			}
		}
	  	newfile.close(); 
	}

	else cout<<"Error file not opening"<<endl;

}
