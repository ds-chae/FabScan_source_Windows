#include "atfz.h"
#include <vector>

atfz::atfz(void)
{
}


atfz::~atfz(void)
{
}





  void atfz::setImage(cv::Mat image)
  {
    this->image = image;
	
	this->imageMatrix= image.clone();

   // this->imageMatrix = new short[image.cols][image.rows];

	//	int row= image.rows;
	//	int col =image.cols;
	////使用数组指针
 //   //分配一个指针数组,其首地址保存在b中
 //  // short **
	//	imageMatrix=new short*[col];

 //   //为指针数组的每个元素分配一个数组
 //   for(int i=0;i<col;i++)
 //      imageMatrix[i]=new short[row];




    //for (int i = 0; i < image.cols; i++)
    //  for (int j = 0; j < image.rows; j++)
    //    if ( image.at<int>(j, i) == 0)
    //    {
    //      this->imageMatrix.at<int>( j, i) = 1;
    //    }
    //    else
    //      this->imageMatrix.at<int>( j, i) = 0;

	  cv::imshow("imageMatriZ34", imageMatrix);  cv::waitKey(3000);

  }

  void atfz::updateImage()
  {
    //for (int i = 0; i < this->image.rows; i++)
    //  for (int j = 0; j < this->image.cols; j++)
    //  {
    //    short pixel_value;
    //   // short pixel_value;
    //    if (this->imageMatrix.at<int>( i, j) == 0)
    //      pixel_value = 255;
    //    else {
    //      pixel_value = 0;
    //    }

    //   // Scalar    FACE_RECT_COLOR1     = new Scalar(pixel_value, pixel_value, pixel_value, pixel_value);
    //  //  double[] x={pixel_value};
    //    this->image.at<int>( i, j) =  pixel_value;
    //  }


	  this->image= imageMatrix.clone();

  }

   cv::Mat atfz::getImage()
  {
    return this->image;
  }

   cv::Mat atfz::getThinnedImage() {
   
	    cv::imshow("step_imageZ34_1", this->image);  cv::waitKey(1000);

	   thin();

	 cv::imshow("step_imageZ34_2", this->image);  cv::waitKey(1000);


    updateImage();

	 cv::imshow("step_imageZ34_3", this->image);  cv::waitKey(1000);

    return this->image;
  }

  short atfz::mappingFunction(short i, short j)
  {
    if ((i == 0) && (j == 0)) {
      return 0;
    }

    if ((i == -1) && (j == -1)) {
      return 1;
    }

    if ((i == -1) && (j == 0)) {
      return 2;
    }

    if ((i == -1) && (j == 1)) {
      return 3;
    }

    if ((i == 0) && (j == 1)) {
      return 4;
    }

    if ((i == 1) && (j == 1)) {
      return 5;
    }

    if ((i == 1) && (j == 0)) {
      return 6;
    }

    if ((i == 1) && (j == -1)) {
      return 7;
    }

    if ((i == 0) && (j == -1)) {
      return 8;
    }

    return -1;
  }

  cv::Point atfz::reverseMappingFunction(int index) {
    switch (index) {
    case 0:
      return  cv::Point(0, 0);
    case 1:
      return  cv::Point(-1, -1);
    case 2:
      return  cv::Point(-1, 0);
    case 3:
      return  cv::Point(-1, 1);
    case 4:
      return  cv::Point(0, 1);
    case 5:
      return  cv::Point(1, 1);
    case 6:
      return  cv::Point(1, 0);
    case 7:
      return  cv::Point(1, -1);
    case 8:
      return  cv::Point(0, -1);
    }

    return  cv::Point(-2, -2);
  }

  int atfz::Cs(cv::Point p) {
    cv::Point p1 = reverseMappingFunction(1);
    cv::Point p2 = reverseMappingFunction(2);
    cv::Point p3 = reverseMappingFunction(3);
    cv::Point p4 = reverseMappingFunction(4);
    cv::Point p5 = reverseMappingFunction(5);
    cv::Point p6 = reverseMappingFunction(6);
    cv::Point p7 = reverseMappingFunction(7);
    cv::Point p8 = reverseMappingFunction(8);

    int p1_value = this->imageMatrix.at<int>((int) (p.y - p1.y) ,(int) (p.x - p1.x) );//[][];
    int p2_value = this->imageMatrix.at<int>((int) (p.y - p2.y) ,(int) (p.x - p2.x) );//[][];
    int p3_value = this->imageMatrix.at<int>((int) (p.y - p3.y) ,(int) (p.x - p3.x) );//[][];
    int p4_value = this->imageMatrix.at<int>((int) (p.y - p4.y) ,(int) (p.x - p4.x) );//[][];
    int p5_value = this->imageMatrix.at<int>((int) (p.y - p5.y) ,(int) (p.x - p5.x) );//[][];
    int p6_value = this->imageMatrix.at<int>((int) (p.y - p6.y) ,(int) (p.x - p6.x) );//[][];
    int p7_value = this->imageMatrix.at<int>((int) (p.y - p7.y) ,(int) (p.x - p7.x) );//[][];
    int p8_value = this->imageMatrix.at<int>((int) (p.y - p8.y) ,(int) (p.x - p8.x) );//[][];

    int expression = (1 - p2_value & (p3_value | p4_value)) + (1 - p4_value & (p5_value | p6_value)) + (1 - p6_value & (p7_value | p8_value)) + (1 - p8_value & (p1_value | p2_value));

    return expression;
  }

  int atfz::Ns(cv::Point p) {
    cv::Point p1 = reverseMappingFunction(1);
    cv::Point p2 = reverseMappingFunction(2);
    cv::Point p3 = reverseMappingFunction(3);
    cv::Point p4 = reverseMappingFunction(4);
    cv::Point p5 = reverseMappingFunction(5);
    cv::Point p6 = reverseMappingFunction(6);
    cv::Point p7 = reverseMappingFunction(7);
    cv::Point p8 = reverseMappingFunction(8);

    int p1_value = this->imageMatrix.at<int>((int) (p.y - p1.y) ,(int) (p.x - p1.x) );//[][];
    int p2_value = this->imageMatrix.at<int>((int) (p.y - p2.y) ,(int) (p.x - p2.x) );//[][];
    int p3_value = this->imageMatrix.at<int>((int) (p.y - p3.y) ,(int) (p.x - p3.x) );//[][];
    int p4_value = this->imageMatrix.at<int>((int) (p.y - p4.y) ,(int) (p.x - p4.x) );//[][];
    int p5_value = this->imageMatrix.at<int>((int) (p.y - p5.y) ,(int) (p.x - p5.x) );//[][];
    int p6_value = this->imageMatrix.at<int>((int) (p.y - p6.y) ,(int) (p.x - p6.x) );//[][];
    int p7_value = this->imageMatrix.at<int>((int) (p.y - p7.y) ,(int) (p.x - p7.x) );//[][];
    int p8_value = this->imageMatrix.at<int>((int) (p.y - p8.y) ,(int) (p.x - p8.x) );//[][];

    int N1 = (p1_value | p2_value) + (p3_value | p4_value) + (p5_value | p6_value) + (p7_value | p8_value);
    int N2 = (p2_value | p3_value) + (p4_value | p5_value) + (p6_value | p7_value) + (p8_value | p1_value);

    if (N1 < N2) {
      return N1;
    }
    return N2;
  }

  bool atfz::condition_3(cv::Point p, int iteration_number)
  {
    cv::Point p1 = reverseMappingFunction(1);
    cv::Point p2 = reverseMappingFunction(2);
    cv::Point p3 = reverseMappingFunction(3);
    cv::Point p4 = reverseMappingFunction(4);
    cv::Point p5 = reverseMappingFunction(5);
    cv::Point p6 = reverseMappingFunction(6);
    cv::Point p7 = reverseMappingFunction(7);
    cv::Point p8 = reverseMappingFunction(8);

    int p1_value = this->imageMatrix.at<int>((int) (p.y - p1.y) ,(int) (p.x - p1.x) );//[][];
    int p2_value = this->imageMatrix.at<int>((int) (p.y - p2.y) ,(int) (p.x - p2.x) );//[][];
    int p3_value = this->imageMatrix.at<int>((int) (p.y - p3.y) ,(int) (p.x - p3.x) );//[][];
    int p4_value = this->imageMatrix.at<int>((int) (p.y - p4.y) ,(int) (p.x - p4.x) );//[][];
    int p5_value = this->imageMatrix.at<int>((int) (p.y - p5.y) ,(int) (p.x - p5.x) );//[][];
    int p6_value = this->imageMatrix.at<int>((int) (p.y - p6.y) ,(int) (p.x - p6.x) );//[][];
    int p7_value = this->imageMatrix.at<int>((int) (p.y - p7.y) ,(int) (p.x - p7.x) );//[][];
    int p8_value = this->imageMatrix.at<int>((int) (p.y - p8.y) ,(int) (p.x - p8.x) );//[][];
    bool toReturn;
   // bool toReturn;
    if (iteration_number % 2 == 0)
      toReturn = ((p2_value | p3_value | 1 - p5_value) & p4_value) == 0;
    else {
      toReturn = ((p6_value | p7_value | 1 - p1_value) & p8_value) == 0;
    }

    return toReturn;
  }

  int atfz::C(cv::Point p) {
    cv::Point p1 = reverseMappingFunction(1);
    cv::Point p2 = reverseMappingFunction(2);
    cv::Point p3 = reverseMappingFunction(3);
    cv::Point p4 = reverseMappingFunction(4);
    cv::Point p5 = reverseMappingFunction(5);
    cv::Point p6 = reverseMappingFunction(6);
    cv::Point p7 = reverseMappingFunction(7);
    cv::Point p8 = reverseMappingFunction(8);

    int p1_value = this->imageMatrix.at<int>((int) (p.y - p1.y) ,(int) (p.x - p1.x) );//[][];
    int p2_value = this->imageMatrix.at<int>((int) (p.y - p2.y) ,(int) (p.x - p2.x) );//[][];
    int p3_value = this->imageMatrix.at<int>((int) (p.y - p3.y) ,(int) (p.x - p3.x) );//[][];
    int p4_value = this->imageMatrix.at<int>((int) (p.y - p4.y) ,(int) (p.x - p4.x) );//[][];
    int p5_value = this->imageMatrix.at<int>((int) (p.y - p5.y) ,(int) (p.x - p5.x) );//[][];
    int p6_value = this->imageMatrix.at<int>((int) (p.y - p6.y) ,(int) (p.x - p6.x) );//[][];
    int p7_value = this->imageMatrix.at<int>((int) (p.y - p7.y) ,(int) (p.x - p7.x) );//[][];
    int p8_value = this->imageMatrix.at<int>((int) (p.y - p8.y) ,(int) (p.x - p8.x) );//[][];

    int expression = (p2_value | p4_value | p6_value | p8_value) + (1 - p2_value & 1 - p4_value & p3_value) + (1 - p4_value & 1 - p6_value & p5_value) + (1 - p6_value & 1 - p8_value & p7_value) + (1 - p8_value & 1 - p2_value & p1_value) + (1 - p4_value & 1 - p8_value & p2_value & p6_value) + (1 - p2_value & 1 - p6_value & p4_value & p8_value);

    return expression;
  }

  bool atfz::N(cv::Point p) {
    cv::Point p1 = reverseMappingFunction(1);
    cv::Point p2 = reverseMappingFunction(2);
    cv::Point p3 = reverseMappingFunction(3);
    cv::Point p4 = reverseMappingFunction(4);
    cv::Point p5 = reverseMappingFunction(5);
    cv::Point p6 = reverseMappingFunction(6);
    cv::Point p7 = reverseMappingFunction(7);
    cv::Point p8 = reverseMappingFunction(8);

    int p1_value = this->imageMatrix.at<int>((int) (p.y - p1.y) ,(int) (p.x - p1.x) );//[][];
    int p2_value = this->imageMatrix.at<int>((int) (p.y - p2.y) ,(int) (p.x - p2.x) );//[][];
    int p3_value = this->imageMatrix.at<int>((int) (p.y - p3.y) ,(int) (p.x - p3.x) );//[][];
    int p4_value = this->imageMatrix.at<int>((int) (p.y - p4.y) ,(int) (p.x - p4.x) );//[][];
    int p5_value = this->imageMatrix.at<int>((int) (p.y - p5.y) ,(int) (p.x - p5.x) );//[][];
    int p6_value = this->imageMatrix.at<int>((int) (p.y - p6.y) ,(int) (p.x - p6.x) );//[][];
    int p7_value = this->imageMatrix.at<int>((int) (p.y - p7.y) ,(int) (p.x - p7.x) );//[][];
    int p8_value = this->imageMatrix.at<int>((int) (p.y - p8.y) ,(int) (p.x - p8.x) );//[][];

    int N4 = p2_value + p4_value + p6_value + p8_value;

    bool A = p1_value + p5_value == 1;
    bool B = p3_value + p7_value == 1;

    if ((N4 == 1) && (Ns(p) == 3) && ((A) || (B))) {
      return true;
    }
    return false;
  }

  void atfz::thin()
  {
    int iteration_number = 0;
    bool pixels_deleted;

	// imageMatrix=image.clone();

    do
    {
      iteration_number++;
      pixels_deleted = false;

	  std::vector<  cv::Point> flagList;
   
	 

     // ArrayList flagList = new ArrayList();

      for (int i = 1; i < this->image.rows - 1; i++) {
        for (int j = 1; j < this->image.cols-1; j++) {
          cv::Point p =   cv::Point(j, i);

          bool is_black =( this->imageMatrix.at<int>(i,j)==0);

          if (is_black)
          {
            bool C_condition = (C(p) == 1);
            bool Ns_condition = (Ns(p) >= 2) && (Ns(p) <= 3);
            bool condition3 = condition_3(p, iteration_number);

            if (!((C_condition) && (Ns_condition) && (condition3)))
            {
              if (!N(p)) {
                flagList.push_back(p);
                pixels_deleted = true;
              }
            }
          }
        }

      }

	  	 cv::imshow("step_imageZ31imageMatrix1", this->imageMatrix);  cv::waitKey(10);



      for (int index = 0; index < flagList.size(); index++) {
        cv::Point flagged_point = (cv::Point)flagList[index];
        this->imageMatrix.at<int>( (int) flagged_point.y  ,(int) flagged_point.x) = 0;
      }

	  	 cv::imshow("step_imageZ31imageMatrix2", this->imageMatrix);  cv::waitKey(10);


    }
    while (pixels_deleted);

    this->updateImage();
  }

  int atfz::blackToWhiteTransitionCount(cv::Point p) {
    short counter = 0;
    for (int index = 1; index < 9; index++) {
      cv::Point first_point = reverseMappingFunction(index);
      bool first_condition =( this->imageMatrix.at<int>( (int) (p.y + first_point.y), (int) (p.x + first_point.x))  * this->imageMatrix.at<int>((int) (p.y + first_point.y) ,(int) (p.x + first_point.x) )==0);//  [][] == 0;
      cv::Point second_point;
     // Point second_point;
      if (index + 1 == 9)
        second_point = reverseMappingFunction(1);
      else {
        second_point = reverseMappingFunction(index + 1);
      }
      bool second_condition =( this->imageMatrix.at<int>( (int) (p.y + first_point.y), (int) (p.x + first_point.x))  * this->imageMatrix.at<int>((int) (p.y + first_point.y) ,(int) (p.x + first_point.x) )==1);//  [][] == 0;
  
		  // this->imageMatrix[(int) (p.x + second_point.x)][(int) (p.y + second_point.y)] * this->imageMatrix[(int) (p.x + second_point.x)][(int) (p.y + second_point.y)] == 1;

      if ((first_condition) && (second_condition)) {
        counter = (short)(counter + 1);
      }
    }

    return counter;
  }

  double atfz::sensitivityMeasurement() {
    double SM = 0 ;
    double total_black_pixel_count = 1.0 ;

    for (int i = 1; i < this->image.rows - 1; i++) {
      for (int j = 1; j < this->image.cols; j++) {
        if (this->imageMatrix.at<int>(i,j) == 1) {
          total_black_pixel_count += 1.0;
          if (blackToWhiteTransitionCount(  cv::Point(i, j)) > 2) {
            SM += 1.0;
          }
        }
      }
    }

    return SM / total_black_pixel_count;
  }
 