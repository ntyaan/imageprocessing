#include <iostream>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <vector>
template <class T>
class Array3{
private:
  std::vector<std::vector<unsigned char> > org_image_gray;
  std::vector<std::vector<unsigned char> > image_gray;
  std::vector<std::vector<unsigned char> > tmp_image;
  int SEEK;
  int ROW, COL, DEPTH;
public:
  Array3(int row, int col, int depth, int seek):
    SEEK(seek), ROW(row), COL(col), DEPTH(depth){
    org_image_gray.resize(ROW);
    tmp_image.resize(ROW);
    image_gray.resize(ROW);
    for(int i=0;i<ROW;i++){
      org_image_gray[i].resize(COL);
      tmp_image[i].resize(COL);
      image_gray[i].resize(COL);
    }
  }
  //デストラクタ
  ~Array3(void){
    for(int i=0;i<ROW;i++){
      org_image_gray[i].shrink_to_fit();
      tmp_image[i].shrink_to_fit();
      image_gray[i].shrink_to_fit();
    }
  }
  //コピー代入
  Array3& operator=(const Array3 &arg){
    if(this->ROW!=arg.ROW || this->COL!=arg.COL){
      std::cerr<<"operator= error"<<std::endl;
      exit(1);
    }
    for(int i=0;i<ROW;i++)
      for(int j=0;j<COL;j++){
	this->org_image_gray[i][j]=arg.org_image_gray[i][j];
	this->tmp_image[i][j]=arg.tmp_image[i][j];
	this->image_gray[i][j]=arg.image_gray[i][j];
      }
    this->SEEK=arg.SEEK;
    return *this;
  }
  //高さを返す
  int row(){
    return ROW;
  }
  //幅を返す
  int col(){
    return COL;
  }
  //RGMカラーを返す
  int dep(){
    return DEPTH;
  }
  T operator()(int i,int j) const{
    return tmp_image[i][j];
  }
  T& operator()(int i,int j){
    return tmp_image[i][j];
  }
  
  //画像読み込み
  void input(std::string filename){
    std::ifstream ifs(filename.c_str(),
		      std::ios::in | std::ios::binary);
    if (ifs.fail()){
      std::cerr << "can not input" << std::endl;
      exit(1);
    }
    //ポインタの位置を移動
    ifs.seekg(SEEK);
    //読み込みとgrayscale化
    double gray_var[3]={0.2126, 0.7152, 0.0722};
    for(int i=0;i<ROW;i++){
      for(int j=0;j<COL;j++){
	unsigned char tmp;
	int var=0;
	for(int k=0;k<DEPTH;k++){
	  ifs.read(reinterpret_cast<char*>(&tmp), 1);
	  var+=tmp*gray_var[k];
	}
	org_image_gray[i][j]=var;
	image_gray[i][j]=var;
	tmp_image[i][j]=var;
      }
    }
    ifs.close();
    return;
  }
  
  //画像出力
  void output(std::string filename){
    std::ofstream ofs(filename.c_str(),
		      std::ios::out | std::ios::binary | std::ios::trunc);
    if (!ofs){
      std::cerr << "can not output" << std::endl;
      exit(1);
    }
    std::string str
      = "P6\n" + std::to_string(COL)
      + " " + std::to_string(ROW) + "\n255\nr";
    ofs.write(str.c_str(), str.size()-1);
    for(int i=0;i<ROW;i++)
      for(int j=0;j<COL;j++)
	for(int k=0;k<DEPTH;k++)
	  ofs.write(reinterpret_cast<char*>(&tmp_image[i][j]), 1);
    ofs.close();
    return;
  }
  
  void reset(){
    for(int i=0;i<ROW;i++)
      for(int j=0;j<COL;j++){
	image_gray[i][j]=org_image_gray[i][j];
      }
    return;
  }
  
  //tmp_iamge=image_gray
  void gray(){
    for(int i=0;i<ROW;i++)
      for(int j=0;j<COL;j++)
	image_gray[i][j]=tmp_image[i][j];
    return;
  }
  
  //MAX-MINフィルタ
  void max_min(){
    //中央
    this->simple_four_edge();
    for(int i=1;i<ROW-1;i++){
      for(int j=1;j<COL-1;j++){
	int min=0xff, max=0x00;
	for(int x=i-1;x<i-1+3;x++){
	  for(int y=j-1;y<j-1+3;y++){
	    if(image_gray[x][y]<min)
	      min=image_gray[x][y];
	    if(image_gray[x][y]>max)
	      max=image_gray[x][y];
	  }
	}
	tmp_image[i][j]=abs(max-min);
      }
    }
    return;
  }

  //グレースケール化されたものを単純に2値化
  void simple_black_or_whote(){
    for(int i=0;i<ROW;i++){ 
      for(int j=0;j<COL;j++){
	unsigned char tmp;
	if(image_gray[i][j]>0x80)
	  tmp=0xff;
	else
	  tmp=0x00;
	tmp_image[i][j]=tmp;
      }
    }
    this->gray();
    return;
  }
  
  //白黒二値化されている画像に対して
  //対象画素とその周り8マスにおいて
  //白黒で多い方を採用する
  void simple_get_edge(){
    this->simple_black_or_whote();
    this->four_edge_for_edge();
    //中央
    for(int i=1;i<ROW-1;i++){
      for(int j=1;j<COL-1;j++){
	int tmp=0;
	for(int x=i-1;x<i-1+3;x++){
	  for(int y=j-1;y<j-1+3;y++){
	    tmp+=image_gray[x][y];
	  }
	}
	if((0xff*9/2)<tmp)
	  tmp_image[i][j]=0xff;
	else
	  tmp_image[i][j]=0x00;
      }
    }
    this->reset();
    return;
  }

  //ソーベル縦フィルタ
  void sorbel_vertical_filter(){
    //中央
    this->simple_four_edge();
    for(int i=1;i<ROW-1;i++){
      for(int j=1;j<COL-1;j++){
	int tmp=0;
	tmp+=image_gray[i-1][j-1]
	  +image_gray[i-1][j+1]*(-1)
	  +image_gray[i][j-1]*2
	  +image_gray[i][j+1]*(-2)
	  +image_gray[i+1][j-1]
	  +image_gray[i+1][j+1]*(-1);
	if(tmp>0x00)
	  tmp_image[i][j]=tmp;
	else
	  tmp_image[i][j]=0x00;
      }
    }
    return;
  }

  //ソーベル横フィルタ
  void sorbel_horizontal_filter(){
    //中央
    this->simple_four_edge();
    for(int i=1;i<ROW-1;i++){
      for(int j=1;j<COL-1;j++){
	int tmp=0;
	tmp+=image_gray[i-1][j-1]
	  +image_gray[i-1][j]*2
	  +image_gray[i-1][j+1]
	  +image_gray[i+1][j-1]*(-1)
	  +image_gray[i+1][j]*(-2)
	  +image_gray[i+1][j+1]*(-1);
	if(tmp>0x00)
	  tmp_image[i][j]=tmp;
	else
	  tmp_image[i][j]=0x00;
      }
    }
    return;
  }

  //ソーベルフィルタ
  void sorbel_filter(){
    //中央
    this->simple_four_edge();
    for(int i=1;i<ROW-1;i++){
      for(int j=1;j<COL-1;j++){
	int tmp1=0;
	tmp1+=image_gray[i-1][j-1]
	  +image_gray[i-1][j+1]*(-1)
	  +image_gray[i][j-1]*2
	  +image_gray[i][j+1]*(-2)
	  +image_gray[i+1][j-1]
	  +image_gray[i+1][j+1]*(-1);
	int tmp2=0;
	tmp2+=image_gray[i-1][j-1]
	  +image_gray[i-1][j]*2
	  +image_gray[i-1][j+1]
	  +image_gray[i+1][j-1]*(-1)
	  +image_gray[i+1][j]*(-2)
	  +image_gray[i+1][j+1]*(-1);
	tmp_image[i][j]=sqrt(pow(tmp1,2)+pow(tmp2,2));
      }
    }
    return;
  }

  //4辺の設定
  void four_edge(){
    //四つ角
    tmp_image[0][0]
      =(org_image_gray[0][0]+org_image_gray[1][0]
	+org_image_gray[0][1]+org_image_gray[1][1])/4;
    tmp_image[ROW-1][0]
      =(org_image_gray[ROW-1][0]+org_image_gray[ROW-2][0]
	+org_image_gray[ROW-1][1]+org_image_gray[ROW-2][1])/4;
    tmp_image[0][COL-1]
      =(org_image_gray[0][COL-1]+org_image_gray[1][COL-1]
	+org_image_gray[0][COL-2]+org_image_gray[1][COL-2])/4;
    tmp_image[ROW-1][COL-1]
      =(org_image_gray[ROW-1][COL-1]+org_image_gray[ROW-2][COL-1]
	+org_image_gray[ROW-1][COL-2]+org_image_gray[ROW-2][COL-2])/4;
    //四辺
    for(int i=1;i<ROW-1;i++){
      int tmp1=0,tmp2=0;
      for(int j=i-1;j<i-1+3;j++){
	tmp1+=(org_image_gray[j][0]+org_image_gray[j][1]);
	tmp2+=(org_image_gray[j][COL-1]+org_image_gray[j][COL-2]);
      }
      tmp_image[i][0]=tmp1/6;
      tmp_image[i][COL-1]=tmp2/6;
    }
    for(int i=1;i<COL-1;i++){
      int tmp1=0,tmp2=0;
      for(int j=i-1;j<i-1+3;j++){
	tmp1+=(org_image_gray[0][j]+org_image_gray[1][j]);
	tmp2+=(org_image_gray[ROW-1][j]+org_image_gray[ROW-2][j]);
      }
      tmp_image[0][i]=tmp1/6;
      tmp_image[ROW-1][i]=tmp2/6;
    }
    return;
  }
  
  //グレースケール化されたものを膨張
  void gray_exp(){
    this->four_edge();
    //中央
    for(int i=1;i<ROW-1;i++){
      for(int j=1;j<COL-1;j++){
	int tmp=0;
	for(int x=i-1;x<i-1+3;x++){
	  for(int y=j-1;y<j-1+3;y++){
	    if(tmp<image_gray[x][y])
	      tmp=image_gray[x][y];
	  }
	}
        tmp_image[i][j]=tmp;
      }
    }
    return;
  }

  //グレースケール化されたものを縮小
  void gray_red(){
    this->four_edge();
    //中央
    for(int i=1;i<ROW-1;i++){
      for(int j=1;j<COL-1;j++){
	int tmp=0xff;
	for(int x=i-1;x<i-1+3;x++){
	  for(int y=j-1;y<j-1+3;y++){
	    if(tmp>image_gray[x][y])
	      tmp=image_gray[x][y];
	  }
	}
        tmp_image[i][j]=tmp;
      }
    }
    return;
  }
  
  //グレースケール化されたものを平均フィルタへ
  void average_filter(){
    this->four_edge();
    //中央
    for(int i=1;i<ROW-1;i++){
      for(int j=1;j<COL-1;j++){
	int tmp=0;
	for(int x=i-1;x<i-1+3;x++){
	  for(int y=j-1;y<j-1+3;y++){
	    tmp+=org_image_gray[x][y];
	  }
	}
        tmp_image[i][j]=tmp/9;
      }
    }
    this->gray();
    return;
  }

  //グレースケール化されたものをガウシアンフィルタへ
  void gauss_filter(){
    this->four_edge();
    //中央
    for(int i=1;i<ROW-1;i++){
      for(int j=1;j<COL-1;j++){
	int tmp=(org_image_gray[i][j]*4)/16;
	for(int x=i-1;x<i-1+3;x++)
	  tmp+=(org_image_gray[x][j+1]*2)/16;
	for(int y=j-1;y<j-1+3;y++)
	  tmp+=(org_image_gray[i+1][y]*2)/16;
	tmp+=org_image_gray[i-1][j-1]/16;
	tmp+=org_image_gray[i+1][j-1]/16;
	tmp+=org_image_gray[i-1][j+1]/16;
	tmp+=org_image_gray[i+1][j+1]/16;
        tmp_image[i][j]=tmp;
      }
    }
    this->gray();
    return;
  }

  //グレースケール化されたものをソートフィルタへ
  void sort_filter(){
    this->four_edge();
    //中央
    for(int i=1;i<ROW-1;i++){
      for(int j=1;j<COL-1;j++){
	//クイックソート
	int left=0, right=8, z=0;
	unsigned char tmp[9];
	for(int x=i-1;x<i-1+3;x++){
	  for(int y=j-1;y<j-1+3;y++){
	    tmp[z]=org_image_gray[x][y];
	    z++;
	  }
	}
	unsigned char pivot=tmp[4];
	while(1){
	  while(tmp[left]<pivot)
	    left++;
	  while(pivot<tmp[right])
	    right--;
	  if(left>=right)
	    break;
	  unsigned char temp;
	  temp=tmp[left];
	  tmp[left]=tmp[right];
	  tmp[right]=temp;
	  left++;
	  right--;
	}
	tmp_image[i][j]=tmp[4];
      }
    }
    this->gray();
    return;
  }

  //edge検出用の4辺の設定
  void four_edge_for_edge(){
    //四つ角
    int tmp
      =(image_gray[0][0]+image_gray[1][0]
	+image_gray[0][1]+image_gray[1][1]);
    if(0xff*2<tmp)
      tmp_image[0][0]=0xff;
    else
      tmp_image[0][0]=0x00;

    tmp=(image_gray[ROW-1][0]+image_gray[ROW-2][0]
	 +image_gray[ROW-1][1]+image_gray[ROW-2][1]);
    if(0xff*2<tmp)
      tmp_image[ROW-1][0]=0xff;
    else
      tmp_image[ROW-1][0]=0x00;

    tmp
      =(image_gray[0][COL-1]+image_gray[1][COL-1]
	+image_gray[0][COL-2]+image_gray[1][COL-2]);
    if(0xff*2<tmp)
      tmp_image[0][COL-1]=0xff;
    else
      tmp_image[0][COL-1]=0x00;
   
    tmp
      =(image_gray[ROW-1][COL-1]+image_gray[ROW-2][COL-1]
	+image_gray[ROW-1][COL-2]+image_gray[ROW-2][COL-2]);
    if(0xff*2<tmp)
      tmp_image[ROW-1][COL-1]=0xff;
    else
      tmp_image[ROW-1][COL-1]=0x00;
    //四辺
    for(int i=1;i<ROW-1;i++){
      int tmp1=0,tmp2=0;
      for(int j=i-1;j<i-1+3;j++){
	tmp1+=(image_gray[j][0]+image_gray[j][1]);
	tmp2+=(image_gray[j][COL-1]+image_gray[j][COL-2]);
      } 
      if(0xff*3<tmp1)
	tmp_image[i][0]=0xff;
      else
	tmp_image[i][0]=0x00;      
      if(0xff*3<tmp2)
	tmp_image[i][COL-1]=0xff;
      else
        tmp_image[i][COL-1]=0x00;
    }
    
    for(int i=1;i<COL-1;i++){
      int tmp1=0,tmp2=0;
      for(int j=i-1;j<i-1+3;j++){
	tmp1+=(image_gray[0][j]+image_gray[1][j]);
	tmp2+=(image_gray[ROW-1][j]+image_gray[ROW-2][j]);
      }
      if(0xff*3<tmp1)
	tmp_image[0][i]=0xff;
      else
	tmp_image[0][i]=0x00;
      if(0xff*3<tmp2)
	tmp_image[ROW-1][i]=0xff;
      else
	tmp_image[ROW-1][i]=0x00;
    }
    return;
  }


  //edge検出用の4辺の設定
  void simple_four_edge(){
    //四つ角
    tmp_image[0][0]=0x00;
    tmp_image[ROW-1][0]=0x00;
    tmp_image[0][COL-1]=0x00;
    tmp_image[ROW-1][COL-1]=0x00;
    //四辺
    for(int i=1;i<ROW-1;i++){
      tmp_image[i][0]=0x00;      
      tmp_image[i][COL-1]=0x00;
    }
    for(int i=1;i<COL-1;i++){
      tmp_image[0][i]=0x00;
      tmp_image[ROW-1][i]=0x00;
    }
    return;
  }
};

//16進数ASCIIを10進数に変換
int hex_dec(const char arg[], int size){
  int result=0;
  int n[size];
  char ca;
  // '0' から '9' の文字なら
  for(int count=0;count<size;count++){
    if ('0' <= arg[count] && arg[count] <= '9')
      n[count] = arg[count] - '0';
    // 'a' から 'f' の文字なら
    else if ('a' <= (ca = tolower(arg[count])) && ca <= 'f')
      n[count] = ca - 'a' + 10;
    else;
  }
  for(int g=0;g<size;g++)
    result+=n[g]*pow(10,size-g-1);
  return result;
}

//画像データから情報を得る
void get_info(std::string filename, int &seek, int &row, int &col){
  std::ifstream ifs(filename.c_str(),
		    std::ios::in | std::ios::binary);
  if (ifs.fail()){
    std::cerr << "can not input" << std::endl;
    exit(1);
  }
  unsigned char tmp;
  int c=0, p=0, size1=0, size2=0;
  char temp1[8], temp2[8];
  seek=0;
  while(ifs.read(reinterpret_cast<char*>(&tmp), sizeof(unsigned char))){
    seek++;
    switch (tmp){
      //1回の目0x0a:マジックナンバー
      //2回目:コメントがあった場合
      //3回目の0x0a:幅と高さ
      //4回目の0x0a:255
    case 0x0a:
      c++;
      p=0;
      break;
      //コメントがあったら
    case 0x23:
      c=0;
      break;
    default:
      //0x020まで幅,0x20から高さ
      if(tmp==0x20)
	p++;
      else if(c==1&&tmp!=0x0a){
	switch (p){
	case 0:
	  temp1[size1]=tmp;
	  size1++;
	  break;
	case 1:
	  temp2[size2]=tmp;
	  size2++;
	  break;
	default:
	  break;
	}
      }
      break;
    }
    //0x0aが3回出てきたら
    if(c==3)
      break;
  }
  //サイズが大き過ぎたら終了する
  if(size1>=8 || size2>=8){
    std::cerr<<"row or col  size > 10^8" <<std::endl;
    exit(1);
  }
  col = hex_dec(temp1, size1);
  row = hex_dec(temp2, size2);
  std::cout<<filename
	   <<" : 幅"<<col<<"ピクセル, "
	   <<"高さ"<<row<<"ピクセル"<<std::endl;
  ifs.close();
  return;
}


