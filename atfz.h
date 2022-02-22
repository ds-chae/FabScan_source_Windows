#pragma once

#include "staticHeaders.h"

class atfz
{
public:
	atfz(void);
	~atfz(void);

	cv::Mat image;
	cv::Mat imageMatrix ;

	void setImage(cv::Mat image);

	void updateImage();

	cv::Mat getImage();

	cv::Mat getThinnedImage();

	short mappingFunction(short i, short j);

	cv::Point reverseMappingFunction(int index) ;

	int Cs(cv::Point p) ;
	int Ns(cv::Point p);
	bool condition_3(cv::Point p, int iteration_number);

	int C(cv::Point p) ;
	bool N(cv::Point p);
	void thin();


	int blackToWhiteTransitionCount(cv::Point p);

	double sensitivityMeasurement();


};

/*


public class zd_thin
{
  Mat image;
  short[][] imageMatrix;

  void setImage(Mat image)
  {
    this.image = image;

    this.imageMatrix = new short[image.height()][image.width()];

    for (int i = 0; i < image.height(); i++)
      for (int j = 0; j < image.width(); j++)
        if ( image.get( i, j)[0] == 0.0D)
        {
          this.imageMatrix[i][j] = 1;
        }
        else
          this.imageMatrix[i][j] = 0;
  }

  void updateImage()
  {
    for (int i = 0; i < this.image.height(); i++)
      for (int j = 0; j < this.image.width(); j++)
      {
        short pixel_value;
       // short pixel_value;
        if (this.imageMatrix[i][j] == 0)
          pixel_value = 255;
        else {
          pixel_value = 0;
        }

       // Scalar    FACE_RECT_COLOR1     = new Scalar(pixel_value, pixel_value, pixel_value, pixel_value);
        double[] x={pixel_value};
        this.image.put( i, j, x);
      }
  }

  public Mat getImage()
  {
    return this.image;
  }

  public Mat getThinnedImage() {
    thin();
    updateImage();
    return this.image;
  }

  short mappingFunction(short i, short j)
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

  Point reverseMappingFunction(int index) {
    switch (index) {
    case 0:
      return new Point(0, 0);
    case 1:
      return new Point(-1, -1);
    case 2:
      return new Point(-1, 0);
    case 3:
      return new Point(-1, 1);
    case 4:
      return new Point(0, 1);
    case 5:
      return new Point(1, 1);
    case 6:
      return new Point(1, 0);
    case 7:
      return new Point(1, -1);
    case 8:
      return new Point(0, -1);
    }

    return new Point(-2, -2);
  }

  int Cs(Point p) {
    Point p1 = reverseMappingFunction(1);
    Point p2 = reverseMappingFunction(2);
    Point p3 = reverseMappingFunction(3);
    Point p4 = reverseMappingFunction(4);
    Point p5 = reverseMappingFunction(5);
    Point p6 = reverseMappingFunction(6);
    Point p7 = reverseMappingFunction(7);
    Point p8 = reverseMappingFunction(8);

    int p1_value = this.imageMatrix[(int) (p.x - p1.x)][(int) (p.y - p1.y)];
    int p2_value = this.imageMatrix[(int) (p.x - p2.x)][(int) (p.y - p2.y)];
    int p3_value = this.imageMatrix[(int) (p.x - p3.x)][(int) (p.y - p3.y)];
    int p4_value = this.imageMatrix[(int) (p.x - p4.x)][(int) (p.y - p4.y)];
    int p5_value = this.imageMatrix[(int) (p.x - p5.x)][(int) (p.y - p5.y)];
    int p6_value = this.imageMatrix[(int) (p.x - p6.x)][(int) (p.y - p6.y)];
    int p7_value = this.imageMatrix[(int) (p.x - p7.x)][(int) (p.y - p7.y)];
    int p8_value = this.imageMatrix[(int) (p.x - p8.x)][(int) (p.y - p8.y)];

    int expression = (1 - p2_value & (p3_value | p4_value)) + (1 - p4_value & (p5_value | p6_value)) + (1 - p6_value & (p7_value | p8_value)) + (1 - p8_value & (p1_value | p2_value));

    return expression;
  }

  int Ns(Point p) {
    Point p1 = reverseMappingFunction(1);
    Point p2 = reverseMappingFunction(2);
    Point p3 = reverseMappingFunction(3);
    Point p4 = reverseMappingFunction(4);
    Point p5 = reverseMappingFunction(5);
    Point p6 = reverseMappingFunction(6);
    Point p7 = reverseMappingFunction(7);
    Point p8 = reverseMappingFunction(8);

    int p1_value = this.imageMatrix[(int) (p.x - p1.x)][(int) (p.y - p1.y)];
    int p2_value = this.imageMatrix[(int) (p.x - p2.x)][(int) (p.y - p2.y)];
    int p3_value = this.imageMatrix[(int) (p.x - p3.x)][(int) (p.y - p3.y)];
    int p4_value = this.imageMatrix[(int) (p.x - p4.x)][(int) (p.y - p4.y)];
    int p5_value = this.imageMatrix[(int) (p.x - p5.x)][(int) (p.y - p5.y)];
    int p6_value = this.imageMatrix[(int) (p.x - p6.x)][(int) (p.y - p6.y)];
    int p7_value = this.imageMatrix[(int) (p.x - p7.x)][(int) (p.y - p7.y)];
    int p8_value = this.imageMatrix[(int) (p.x - p8.x)][(int) (p.y - p8.y)];

    int N1 = (p1_value | p2_value) + (p3_value | p4_value) + (p5_value | p6_value) + (p7_value | p8_value);
    int N2 = (p2_value | p3_value) + (p4_value | p5_value) + (p6_value | p7_value) + (p8_value | p1_value);

    if (N1 < N2) {
      return N1;
    }
    return N2;
  }

  boolean condition_3(Point p, int iteration_number)
  {
    Point p1 = reverseMappingFunction(1);
    Point p2 = reverseMappingFunction(2);
    Point p3 = reverseMappingFunction(3);
    Point p4 = reverseMappingFunction(4);
    Point p5 = reverseMappingFunction(5);
    Point p6 = reverseMappingFunction(6);
    Point p7 = reverseMappingFunction(7);
    Point p8 = reverseMappingFunction(8);

    int p1_value = this.imageMatrix[(int) (p.x - p1.x)][(int) (p.y - p1.y)];
    int p2_value = this.imageMatrix[(int) (p.x - p2.x)][(int) (p.y - p2.y)];
    int p3_value = this.imageMatrix[(int) (p.x - p3.x)][(int) (p.y - p3.y)];
    int p4_value = this.imageMatrix[(int) (p.x - p4.x)][(int) (p.y - p4.y)];
    int p5_value = this.imageMatrix[(int) (p.x - p5.x)][(int) (p.y - p5.y)];
    int p6_value = this.imageMatrix[(int) (p.x - p6.x)][(int) (p.y - p6.y)];
    int p7_value = this.imageMatrix[(int) (p.x - p7.x)][(int) (p.y - p7.y)];
    int p8_value = this.imageMatrix[(int) (p.x - p8.x)][(int) (p.y - p8.y)];
    boolean toReturn;
   // boolean toReturn;
    if (iteration_number % 2 == 0)
      toReturn = ((p2_value | p3_value | 1 - p5_value) & p4_value) == 0;
    else {
      toReturn = ((p6_value | p7_value | 1 - p1_value) & p8_value) == 0;
    }

    return toReturn;
  }

  int C(Point p) {
    Point p1 = reverseMappingFunction(1);
    Point p2 = reverseMappingFunction(2);
    Point p3 = reverseMappingFunction(3);
    Point p4 = reverseMappingFunction(4);
    Point p5 = reverseMappingFunction(5);
    Point p6 = reverseMappingFunction(6);
    Point p7 = reverseMappingFunction(7);
    Point p8 = reverseMappingFunction(8);

    int p1_value = this.imageMatrix[(int) (p.x - p1.x)][(int) (p.y - p1.y)];
    int p2_value = this.imageMatrix[(int) (p.x - p2.x)][(int) (p.y - p2.y)];
    int p3_value = this.imageMatrix[(int) (p.x - p3.x)][(int) (p.y - p3.y)];
    int p4_value = this.imageMatrix[(int) (p.x - p4.x)][(int) (p.y - p4.y)];
    int p5_value = this.imageMatrix[(int) (p.x - p5.x)][(int) (p.y - p5.y)];
    int p6_value = this.imageMatrix[(int) (p.x - p6.x)][(int) (p.y - p6.y)];
    int p7_value = this.imageMatrix[(int) (p.x - p7.x)][(int) (p.y - p7.y)];
    int p8_value = this.imageMatrix[(int) (p.x - p8.x)][(int) (p.y - p8.y)];

    int expression = (p2_value | p4_value | p6_value | p8_value) + (1 - p2_value & 1 - p4_value & p3_value) + (1 - p4_value & 1 - p6_value & p5_value) + (1 - p6_value & 1 - p8_value & p7_value) + (1 - p8_value & 1 - p2_value & p1_value) + (1 - p4_value & 1 - p8_value & p2_value & p6_value) + (1 - p2_value & 1 - p6_value & p4_value & p8_value);

    return expression;
  }

  boolean N(Point p) {
    Point p1 = reverseMappingFunction(1);
    Point p2 = reverseMappingFunction(2);
    Point p3 = reverseMappingFunction(3);
    Point p4 = reverseMappingFunction(4);
    Point p5 = reverseMappingFunction(5);
    Point p6 = reverseMappingFunction(6);
    Point p7 = reverseMappingFunction(7);
    Point p8 = reverseMappingFunction(8);

    int p1_value = this.imageMatrix[(int) (p.x - p1.x)][(int) (p.y - p1.y)];
    int p2_value = this.imageMatrix[(int) (p.x - p2.x)][(int) (p.y - p2.y)];
    int p3_value = this.imageMatrix[(int) (p.x - p3.x)][(int) (p.y - p3.y)];
    int p4_value = this.imageMatrix[(int) (p.x - p4.x)][(int) (p.y - p4.y)];
    int p5_value = this.imageMatrix[(int) (p.x - p5.x)][(int) (p.y - p5.y)];
    int p6_value = this.imageMatrix[(int) (p.x - p6.x)][(int) (p.y - p6.y)];
    int p7_value = this.imageMatrix[(int) (p.x - p7.x)][(int) (p.y - p7.y)];
    int p8_value = this.imageMatrix[(int) (p.x - p8.x)][(int) (p.y - p8.y)];

    int N4 = p2_value + p4_value + p6_value + p8_value;

    boolean A = p1_value + p5_value == 1;
    boolean B = p3_value + p7_value == 1;

    if ((N4 == 1) && (Ns(p) == 3) && ((A) || (B))) {
      return true;
    }
    return false;
  }

  void thin()
  {
    int iteration_number = 0;
    boolean pixels_deleted;
    do
    {
      iteration_number++;
      pixels_deleted = false;

      ArrayList flagList = new ArrayList();

      for (int i = 1; i < this.image.height() - 1; i++) {
        for (int j = 1; j < this.image.width(); j++) {
          Point p = new Point(i, j);

          boolean is_black = this.imageMatrix[i][j] == 1;

          if (is_black)
          {
            boolean C_condition = C(p) == 1;
            boolean Ns_condition = (Ns(p) >= 2) && (Ns(p) <= 3);
            boolean condition3 = condition_3(p, iteration_number);

            if ((C_condition) && (Ns_condition) && (condition3))
            {
              if (!N(p)) {
                flagList.add(p);
                pixels_deleted = true;
              }
            }
          }
        }

      }

      for (int index = 0; index < flagList.size(); index++) {
        Point flagged_point = (Point)flagList.get(index);
        this.imageMatrix[(int) flagged_point.x][(int) flagged_point.y] = 0;
      }
    }
    while (pixels_deleted);

    updateImage();
  }

  int blackToWhiteTransitionCount(Point p) {
    short counter = 0;
    for (int index = 1; index < 9; index++) {
      Point first_point = reverseMappingFunction(index);
      boolean first_condition = this.imageMatrix[(int) (p.x + first_point.x)][(int) (p.y + first_point.y)] * this.imageMatrix[(int) (p.x + first_point.x)][(int) (p.y + first_point.y)] == 0;
      Point second_point;
     // Point second_point;
      if (index + 1 == 9)
        second_point = reverseMappingFunction(1);
      else {
        second_point = reverseMappingFunction(index + 1);
      }
      boolean second_condition = this.imageMatrix[(int) (p.x + second_point.x)][(int) (p.y + second_point.y)] * this.imageMatrix[(int) (p.x + second_point.x)][(int) (p.y + second_point.y)] == 1;

      if ((first_condition) && (second_condition)) {
        counter = (short)(counter + 1);
      }
    }

    return counter;
  }

  double sensitivityMeasurement() {
    double SM = 0.0D;
    double total_black_pixel_count = 1.0D;

    for (int i = 1; i < this.image.height() - 1; i++) {
      for (int j = 1; j < this.image.width(); j++) {
        if (this.imageMatrix[i][j] == 1) {
          total_black_pixel_count += 1.0D;
          if (blackToWhiteTransitionCount(new Point(i, j)) > 2) {
            SM += 1.0D;
          }
        }
      }
    }

    return SM / total_black_pixel_count;
  }
}

*/