#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <vector>

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
    std::cerr << "get_info::ifs error" << std::endl;
    exit(1);
  }
  unsigned char tmp;
  int c=0, p=0, size1=0, size2=0;
  char temp1[8], temp2[8];
  seek=0;
  while(ifs.read(reinterpret_cast<char*>(&tmp), 1)){
    seek++;
    switch (tmp){
      //1回の目0x0a:マジックナンバー
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

constexpr int RGB = 3;
std::string filename="original/Lenna.ppm";

int main(void){
  int row, col, seek;
  //行と列とポインタの移動する位置を得る
  get_info(filename, seek, row, col);
  //C++っぽくVector使う
  std::vector<std::vector<unsigned char> > image;
  image.resize(row);
  for(int i=0;i<row;i++)
    image[i].resize(col);
  //バイナリで読み込み
  std::ifstream ifs(filename.c_str(), std::ios::in | std::ios::binary);
  if (ifs.fail()){
    std::cerr << "ifs error" << std::endl;
    exit(1);
  }
  //ポインタの位置を移動
  ifs.seekg(seek);
  //読み込みとgrayscale化
  double gray_var[3]={0.2126, 0.7152, 0.0722};
  for(int i=0;i<row;i++){
    for(int j=0;j<col;j++){
      unsigned char tmp;
      int var=0;
      for(int k=0;k<RGB;k++){
	ifs.read(reinterpret_cast<char*>(&tmp), 1);
	var+=tmp*gray_var[k];
	}
      image[i][j]=var;
    }
  }
  ifs.close();
  //画像出力
  std::ofstream ofs("grayscale_Lenna",
		    std::ios::out | std::ios::binary | std::ios::trunc);
  if (!ofs){
    std::cerr << "ofs error" << std::endl;
    exit(1);
  }
  //最初に加えるマジックナンバー等
  std::string str
    = "P6\n" + std::to_string(row)
    + " " + std::to_string(col) + "\n255\nr";
  ofs.write(str.c_str(), str.size()-1);
  for(int i=0;i<row;i++)
    for(int j=0;j<col;j++)
	for(int k=0;k<RGB;k++)
	  ofs.write(reinterpret_cast<char*>(&image[i][j]), 1);
  ofs.close();
  return 0;  
}
