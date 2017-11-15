#include "image.hxx"

constexpr int RGB = 3;
std::string filename="original/Lenna.ppm";

int main(){
  int seek,row,col;
  get_info(filename, seek,row,col);
  Array3<unsigned char> im(row, col, RGB, seek);
  //画像を読み込みグレースケール化
  im.input(filename);
  im.output("simple_grayscale.ppm");  
  //ソーベル縦フィルタで白黒2値化
  im.sorbel_vertical_filter();
  im.output("simple_sorbel_vertical.ppm");
  //ソーベル横フィルタで白黒2値化
  im.sorbel_horizontal_filter();
  im.output("simple_sorbel_horizontal.ppm");
  //ソーベルフィルタで白黒2値化
  im.sorbel_filter();
  im.output("simple_sorbel.ppm");
  //グレースケール化されたものを膨張
  im.gray_exp();
  im.output("expansion.ppm");
  //グレースケール化されたものを縮小
  im.gray_red();
  im.output("reduction.ppm");
  //Max-Minフィルタで白黒2値化
  im.max_min();
  im.output("simple_maxmin.ppm");
  //単純に2値化されたものを白黒多数決
  im.simple_get_edge();
  im.output("simple_blackwhite.ppm");
  
  //グレースケール化されたものを平均フィルタへ
  im.average_filter();
  im.output("average_grayscale.ppm");
  //Max-Minフィルタで白黒2値化
  im.max_min();
  im.output("average_maxmin.ppm");
  //平均フィルタで2値化されたものを白黒多数決
  im.simple_get_edge();
  im.output("average_blackwhite.ppm");

  
  //グレースケール化されたものをガウシアンフィルタへ
  im.gauss_filter();
  im.output("gauss_grayscale.ppm");
  //Max-Minフィルタで白黒2値化
  im.max_min();
  im.output("gauss_maxmin.ppm");
  //ガウシアンフィルタで2値化されたものを白黒多数決
  im.simple_get_edge();
  im.output("gauss_blackwhite.ppm");

  
  //グレースケール化されたものをソートフィルタへ
  im.sort_filter();
  im.output("sort_grayscale.ppm");
  //Max-Minフィルタで白黒2値化
  im.max_min();
  im.output("sort_maxmin.ppm");
  //ソートフィルタにかけられたものを白黒多数決
  im.simple_get_edge();
  im.output("sort_blackwhite.ppm");


  return 0;
}
