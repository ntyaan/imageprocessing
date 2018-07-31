# Image Processing
授業の課題で出されたのでメモ．
jpgとかpngとか扱いたかったけどOpenCV等の画像処理ライブラリを使うなとのことで諦めて簡単そうなppmを用いた．
(環境:ubuntu16.04 g++ version5.4.0)

## ppmの中身

画像処理で有名な[Lenna.png](http://optipng.sourceforge.net/pngtech/img/lena.html)をImageMagickでppmに変換し使用する．

```bash
~$ convert Lenna.png Lenna.ppm
```

emacsのhexl-find-fileでバイナリデータを確認．

```bash
M-x hexl-find-file
Filename /Lenna.ppm
```

```emacs:Lenna.ppm
00000000: 5036 0a35 3132 2035 3132 0a32 3535 0ae2  P6.512 512.255..
00000010: 897d e289 7ddf 8985 df88 80e2 8a78 e281  .}..}........x..
00000020: 74e4 8a7b e386 7ce3 8c7f e188 77e4 877e  t..{..|.....w..~
...
```

1行目の0x50[p], 0x36[6]がマジックナンバー．

1つ目の0x0aの後0x35[5], 0x31[1], 0x32[2]が画像の幅．

0x20を挟んで0x35[5], 0x31[1], 0x32[2]が画像の高さ．

2つ目の0x0aのから3つ目の0x0aまでフルカラーの255を示す0x32[2], 0x35[5], 0x35[5]．

3つ目の0x0aからデータ部分．

この画像にはないがコメントがある場合は1つ目の0x0aの後0x23[#]から次の0x0aまでコメントが入る．

## 画像を読み込んでみる

取り敢えず3次元配列用意してifstreamでreadすればいっかってことで最初はこんなの

```cpp
#include <iostream>
#include <fstream>
#include <cstdlib>

int main(void){
  unsigned char image[512][512][3];
  //バイナリで読み込み
  std::ifstream ifs("Lenna.ppm", std::ios::in | std::ios::binary);
  if (ifs.fail()){
    std::cerr << "ifs error" << std::endl;
    exit(1);
  }
  //ポインタの位置を移動
  ifs.seekg(15);
  //データ部分をimageへ
  ifs.read(reinterpret_cast<char*>(&image),  512*512*3);
  ifs.close();
  //最初の3バイト確認
    for(int k=0;k<3;k++)
	std::cout << std::hex
		  << static_cast<int>(image[0][0][k])
		  << "\t";
    std::cout<<std::endl;
    return 0;  
}
```

でもせっかくファイルの最初の行に画像の情報あるのにいちいちピクセル数とseekする位置を指定するの面倒だなってことで書き換えた．


```cpp
constexpr int RGB = 3;
std::string filename = "Lenna.ppm";

int main(void){
  int row, col, seek;
  //行と列とポインタの移動する位置を得る関数を作った
  get_info(filename, seek, row, col);
  //C++っぽくVector使う
  std::vector<std::vector<std::vector<unsigned char> > > image;
  image.resize(row);
  for(int i=0;i<row;i++){
    image[i].resize(col);
      for(int j=0;j<col;j++)
	image[i][j].resize(RGB);
  }
  //バイナリで読み込み
  std::ifstream ifs(filename.c_str(), std::ios::in | std::ios::binary);
  if (ifs.fail()){
    std::cerr << "ifs error" << std::endl;
    exit(1);
  }
  //ポインタの位置を移動
  ifs.seekg(seek);
  //データ部分をimageへ
  for(int i=0;i<row;i++)
    for(int j=0;j<col;j++)
      for(int k=0;k<RGB;k++)
	ifs.read(reinterpret_cast<char*>(&image[i][j][k]),  1);
  //これだとコアダンプ
  //ifs.read(reinterpret_cast<char*>(&image),  row*col*RGB);
  ifs.close();
    return 0;  
}
```

関数[get_info](https://github.com/ntyaan/imageprocessing/blob/master/ImageProcessing/get_info.cxx)はもっとスマートに書ける気がするけど動いたからまぁいいや．

## 画像を出力してみ

3次元配列は無駄に感じたので2次元配列にしてグレースケール化された情報のみ格納する．

```cpp
constexpr int RGB = 3;
std::string filename="Lenna.ppm";

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
```


|元画像|グレースケール化| 
|:--|:--|
|![Lenna.png](https://qiita-image-store.s3.amazonaws.com/0/134663/6b9c2cf7-5291-8159-c73c-e892bb7e218d.png)|![grayscale_Lenna.jpg](https://qiita-image-store.s3.amazonaws.com/0/134663/33ee521c-02e2-0559-8dfc-376ecbcb07b9.jpeg)|

### クラスにしてみる

[https://github.com/ntyaan/imageprocessing/blob/master/ImageProcessing/image.hxx](https://github.com/ntyaan/imageprocessing/blob/master/ImageProcessing/image.hxx)
[https://github.com/ntyaan/imageprocessing/blob/master/ImageProcessing/main.cxx](https://github.com/ntyaan/imageprocessing/blob/master/ImageProcessing/main.cxx)

その他いろいろ試した画像

| 単純に白黒|ガウジアンフィルタで白黒|Max-Minフィルタとかいうの|
|:--|:--|:--|
|![simple_blackwhite.jpg](https://qiita-image-store.s3.amazonaws.com/0/134663/803750ad-b533-10e4-1c65-5904a347afbd.jpeg)|![gauss_blackwhite.jpg](https://qiita-image-store.s3.amazonaws.com/0/134663/32c7ad00-fc6f-8c5b-f71a-2e8ff26c1323.jpeg)|![simple_maxmin.jpg](https://qiita-image-store.s3.amazonaws.com/0/134663/2911cd6f-0fd3-f3b6-377e-246b6877d0f3.jpeg)|

|ソーベル縦|ソーベル横|ソーベル両方|
|:--|:--|:--|
|![simple_sorbel_vertical.jpg](https://qiita-image-store.s3.amazonaws.com/0/134663/97041745-f974-662a-b005-43062edd1824.jpeg)|![simple_sorbel_horizontal.jpg](https://qiita-image-store.s3.amazonaws.com/0/134663/86c40b9a-4cd9-26c9-3609-de9633c112c9.jpeg)|![simple_sorbel.jpg](https://qiita-image-store.s3.amazonaws.com/0/134663/6776070f-fed8-adbe-7c8e-0a2f3f3359ab.jpeg)|
