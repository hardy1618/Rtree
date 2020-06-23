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
		    sprintf(sz, "./Files/%d.txt", i+1);
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
				node[2*d+ 2]=0;
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
		memcpy(&node,&leveldata[i+1][papakanodenumber*nodesize*intsize],nodesize*intsize);
		memcpy(&childnode,&leveldata[i][childkanodenumber*nodesize*intsize],nodesize*intsize);
		loop(j,0,d){
			node[2+2*d+ papamechildnumber*(2*d+1)]=levels[i]-1;
			node[3+2*d +j+ papamechildnumber*(2*d+1)]=childnode[2+j];
			node[3+3*d +j+ papamechildnumber*(2*d+1)]=childnode[2+d+j];
			node[2+j]= min(node[2+j],childnode[2+j]);
			node[2+d+j]=max(node[2+d+j],childnode[2+d+j]);
		}

		memcpy(&leveldata[i+1][papakanodenumber*nodesize*intsize],&node,nodesize*intsize);
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
		memcpy(&node,&leveldata[i+1][papakanodenumber*nodesize*intsize],nodesize*intsize);
		memcpy(&childnode,&leveldata[i][childkanodenumber*nodesize*intsize],nodesize*intsize);
		loop(j,0,d){
			node[2+2*d+ papamechildnumber*(2*d+1)]=levels[i]-1;
			node[3+2*d +j+ papamechildnumber*(2*d+1)]=childnode[2+j];
			node[3+3*d +j+ papamechildnumber*(2*d+1)]=childnode[2+d+j];
			node[2+j]= min(node[2+j],childnode[2+j]);
			node[2+d+j]=max(node[2+d+j],childnode[2+d+j]);
		}
		memcpy(&leveldata[i+1][papakanodenumber*nodesize*intsize],&node,nodesize*intsize);
		// loop(j,0,nodesize) cout<<j<<" = "<<node[j]<<endl;
	}
	if(levels[i]%pagecap == 0){
		cout<<"4"<<endl;
		int pagenumber = levelpages[i].GetPageNum();
		levelfiles[i].MarkDirty(pagenumber);
		levelfiles[i].UnpinPage(pagenumber);
		levelpages[i]=levelfiles[i].NewPage();
		leveldata[i]=levelpages[i].GetData();
	}
	levels[i]++;
}


void bulkload(string location ,int N){
	levelfiles.push_back(fm.CreateFile("./Files/0.txt"));
	levels.push_back(1);
	levelpages.push_back(levelfiles.back().NewPage());
	leveldata.push_back(levelpages.back().GetData());
	loop(i,0,location.size()) sz[i]=location[i];
	FileHandler fh= fm.OpenFile(sz);
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
			memcpy(&leveldata[0][(levels[0]%pagecap)*nodesize*intsize], &node, intsize*nodesize);

			loop(j,0,nodesize) node[j]=0;
			loop(j,0,d){
				node[d+2+j]=INT_MIN;
				node[2+j]= INT_MAX;
			}
			fun(0);
		}
		node[0]=levels[0];
		node[1]=int(levels[0]/maxcap);
		int num;
		loop(j,0,d){
			if(countread==page_size){   //if reader page is full and needs new page
				countread=0;
				int readpagenumber= ph.GetPageNum();
				fh.UnpinPage(readpagenumber);
				ph=fh.NextPage(readpagenumber);
				data=ph.GetData();

			}
            // can reduce the number of memcpy calls
			memcpy (&num, &data[countread], intsize);
			node[d+2+j]=max(node[d+2+j],num);
			node[2+j]=min(node[2+j],num);
			node[2*d+3 + (i%maxcap) *(2*d+1)+j]=num;
			countread+=intsize;
		}

	}
	cout<<levels.size()<<endl;
	loop(i,0,levels.size()){
	    // sprintf(sz, "%d.txt", i);
		// cout<<"destroyed"<<i<<endl;

		// fm.DestroyFile(sz);
        levelfiles[i].UnpinPage(levelpages[i].GetPageNum());
	}
    //unpined all the pages in the buffer after bulkload
}

/* Point query shall be executed as a depth first search, with the pages being unpinned on backtracking.*/

// for (non_leaf) internal nodes
bool iis_contained(vector<int> & point, int* mbrs){
    loop(j,0,d){
        if(mbrs[j]>point[j] || mbrs[d+j]<point[j]) return false;
    }
    return true;
}

// for leaf nodes
bool lis_contained(vector<int> & point, int* node){
    loop(j,0,d){
        if(node[2+j]>point[j] || node[2+d+j]<point[j]) return false;
    }
    loop(i,0,maxcap){
        bool same_point = true;
        loop(j,0,d){
            if(node[2*d+3 + i *(2*d+1)+j]!=point[j]) same_point = false;
        }
        if(same_point) {
            levelfiles[0].UnpinPage(node[0]%pagecap);
            return true;   
        }     
    }
    levelfiles[0].UnpinPage(node[0]%pagecap);
    return false;
}

bool non_leaf_match(vector<int> & point, int* node, int level){
    loop(i, 0, maxcap){
        if(node[2*d+3 + i *(2*d+1)]== INT_MAX || node[2*d+3 + i *(2*d+1)]== INT_MIN) break; //started chechking empty nodes.
        else if(iis_contained(point, node+(2*d+3 + i *(2*d+1)))){
            int* childpos = (node + (2*d+2 + i *(2*d+1)));
            int childnum;
            memcpy(&childnum, childpos, intsize);
            levelpages[level-1] = levelfiles[level-1].PageAt(childnum/pagecap);
            leveldata[level-1]=levelpages[level-1].GetData();
            int childnode[nodesize];
            memcpy(&childnode, &leveldata[level-1][(childnum%pagecap)*nodesize*intsize], intsize*nodesize);
            
            bool is_present;
            if(level==1) //moving to the leaf node next
                if(lis_contained(point, childnode)) is_present = true;
            else
                if(non_leaf_match(point, childnode, level-1)) is_present = true; 
            
            if(is_present){
                levelfiles[level].UnpinPage(node[0]%pagecap);
                return true;
            }
        }
    }
    levelfiles[level].UnpinPage(node[0]%pagecap);
    return false;
}

bool query(vector<int> & point){
    int root_pos = levels.size()-1; //root node is with levels value as 1. 
    int root_node[nodesize];
    leveldata[root_pos]=levelpages[root_pos].GetData();
    memcpy(root_node,&leveldata[root_pos][0],nodesize*intsize);
    if(root_pos==0) return lis_contained(point, root_node);
    if(iis_contained(point, root_node+2))
        return non_leaf_match(point, root_node, root_pos);
    else 
        return false; 
        // no need to unpin the root page as it will mostly be used again.
}

void insert(vector<int> & point){
    
}







int main( int argc, char *argv[]) {
	fstream newfile,file;	
	// loop(i,1,argc) cout<<argv[i]<<endl;
	char* p;
	maxcap = strtol(argv[2], &p, 10);
	page_size= PAGE_CONTENT_SIZE;
	buffersize= BUFFER_SIZE;
	intsize= sizeof(int);
	d=strtol(argv[3], &p, 10);
	nodesize= 2+ 2*d + maxcap*(1+2*d);
	int ae[nodesize];
	// cout<<nodesize<<" "<<sizeof(ae)<<endl;
	pagecap= int(page_size/sizeof(ae));

	// cout<<pagecap<<endl;
	newfile.open(argv[1],ios::in);
    file.open(argv[4],ios::out); 
	if (newfile.is_open() && file.is_open()){ 
		string tp;	
		vector<string> temp;
		while(getline(newfile, tp)){
			temp= split(tp);
			if(temp[0]=="BULKLOAD"){
                file<<"BULKLOAD\n\n";
				bulkload(temp[1],stoi(temp[2]));
				cout<<"tisdfsd"<<endl;
				// test();
			}
			else if(temp[0]=="INSERT"){
                file<<"INSERT\n\n";
                vector<int> point(d,0);
                loop(i, 0, d) 
                    point[i] = stoi(temp[i+1]);
				insert(point);
			}
			else{
                vector<int> point(d,0);
                loop(i, 0, d) 
                    point[i] = stoi(temp[i+1]);
				if(query(point))
                    file<<"TRUE";
                else file<<"FALSE";
                file << "\n\n";
			}
		}
	  	newfile.close(); 
        file.close();
	}

	else cout<<"Error file not opening"<<endl;

}

//levels[i] is the number of child nodes in the i-1 th layer.