#include "atfx.h"
 
#include <stdio.h>
//#include <math.h>

/*
 *  CurveCSS.cpp
 *  CurveMatching
 *
 *  Created by Roy Shilkrot on 11/28/12.
 *
 */

///#include "std.h"

using namespace cv;

//#include "CurveCSS.h"


#pragma mark Gaussian Smoothing and Curvature 

/* 1st and 2nd derivative of 1D gaussian 
 */
void getGaussianDerivs(double sigma, int M, vector<double>& gaussian, vector<double>& dg, vector<double>& d2g) {
	int L = (M - 1) / 2;
	double sigma_sq = sigma * sigma;
	double sigma_quad = sigma_sq*sigma_sq;
	dg.resize(M); d2g.resize(M); gaussian.resize(M);
	
	Mat_<double> g = getGaussianKernel(M, sigma, CV_64F);
	for (double i = -L; i < L+1.0; i += 1.0) {
		int idx = (int)(i+L);
		gaussian[idx] = g(idx);
		// from http://www.cedar.buffalo.edu/~srihari/CSE555/Normal2.pdf
		dg[idx] = (-i/sigma_sq) * g(idx);
		d2g[idx] = (-sigma_sq + i*i)/sigma_quad * g(idx);
	}
}

/* 1st and 2nd derivative of smoothed curve point */
void getdX(vector<double> x, 
		   int n, 
		   double sigma, 
		   double& gx, 
		   double& dgx, 
		   double& d2gx, 
		   vector<double> g, 
		   vector<double> dg, 
		   vector<double> d2g,
		   bool isOpen = false) 
{		
	int L = (g.size() - 1) / 2;

	gx = dgx = d2gx = 0.0;
//	cout << "Point " << n << ": ";
	for (int k = -L; k < L+1; k++) {
		double x_n_k;
		if (n-k < 0) {
			if (isOpen) {
				//open curve - mirror values on border
				x_n_k = x[-(n-k)]; 
			} else {
				//closed curve - take values from end of curve
				x_n_k = x[x.size()+(n-k)];
			}			
		} else if(n-k > x.size()-1) {
			if (isOpen) {
				//mirror value on border
				x_n_k = x[n+k]; 
			} else {
				x_n_k = x[(n-k)-(x.size())];
			}			
		} else {
//			cout << n-k;
			x_n_k = x[n-k];
		}
//		cout << "* g[" << g[k + L] << "], ";

		gx += x_n_k * g[k + L]; //gaussians go [0 -> M-1]
		dgx += x_n_k * dg[k + L]; 
		d2gx += x_n_k * d2g[k + L];
	}
//	cout << endl;
}


/* 0th, 1st and 2nd derivatives of whole smoothed curve */
void getdXcurve(vector<double> x, 
				double sigma, 
				vector<double>& gx, 
				vector<double>& dx, 
				vector<double>& d2x, 
				vector<double> g, 
				vector<double> dg, 
				vector<double> d2g,
				bool isOpen = false) 
{	
	gx.resize(x.size()); 
	dx.resize(x.size()); 
	d2x.resize(x.size());
	for (int i=0; i<x.size(); i++) {
		double gausx,dgx,d2gx; getdX(x,i,sigma,gausx,dgx,d2gx,g,dg,d2g,isOpen);
		gx[i] = gausx;
		dx[i] = dgx;
		d2x[i] = d2gx;
	}
}

void ResampleCurve(const vector<double>& curvex, const vector<double>& curvey,
				   vector<double>& resampleX, vector<double>& resampleY,
				   int N,
				   bool isOpen
				   ) {
	assert(curvex.size()>0 && curvey.size()>0 && curvex.size()==curvey.size());
	
	vector<Point2d> resamplepl(N); resamplepl[0].x = curvex[0]; resamplepl[0].y = curvey[0];
	vector<Point2d> pl; PolyLineMerge(pl,curvex,curvey);

	double pl_length = arcLength(pl, false);
	double resample_size = pl_length / (double)N;
	int curr = 0;
	double dist = 0.0;
	for (int i=1; i<N; ) {		
		assert(curr < pl.size() - 1);
		double last_dist = norm(pl[curr] - pl[curr+1]);
		dist += last_dist;
//		cout << curr << " and " << curr+1 << "\t\t" << last_dist << " ("<<dist<<")"<<endl;
		if (dist >= resample_size) {
			//put a point on line
			double _d = last_dist - (dist-resample_size);
			Point2d cp(pl[curr].x,pl[curr].y),cp1(pl[curr+1].x,pl[curr+1].y);
			Point2d dirv = cp1-cp; dirv = dirv * (1.0 / norm(dirv));
//			cout << "point " << i << " between " << curr << " and " << curr+1 << " remaining " << dist << endl;
			assert(i < resamplepl.size());
			resamplepl[i] = cp + dirv * _d;
			i++;
			
			dist = last_dist - _d; //remaining dist			
			
			//if remaining dist to next point needs more sampling... (within some epsilon)
			while (dist - resample_size > 1e-3) {
//				cout << "point " << i << " between " << curr << " and " << curr+1 << " remaining " << dist << endl;
				assert(i < resamplepl.size());
				resamplepl[i] = resamplepl[i-1] + dirv * resample_size;
				dist -= resample_size;
				i++;
			}
		}
		
		curr++;
	}
	
	PolyLineSplit(resamplepl,resampleX,resampleY);
}

#pragma mark CSS image

int roundx(double f)
{ 
if ((int)f+0.5>f) 
return (int)f; 
else 
return (int)f + 1;   
}


/* compute curvature of curve after gaussian smoothing 
 from "Shape similarity retrieval under affine transforms", Mokhtarian & Abbasi 2002
 curvex - x position of points
 curvey - y position of points
 kappa - curvature coeff for each point
 sigma - gaussian sigma
 */
void ComputeCurveCSS(const vector<double>& curvex, 
					 const vector<double>& curvey, 
					 vector<double>& kappa, 
					 vector<double>& smoothX, vector<double>& smoothY,
					 double sigma,
					 bool isOpen
					 ) 
{
	int M =  roundx((10.0*sigma+1.0) / 2.0) * 2 - 1;
	assert(M % 2 == 1); //M is an odd number
	
	vector<double> g,dg,d2g; getGaussianDerivs(sigma,M,g,dg,d2g);
	
	vector<double> X,XX,Y,YY;
	getdXcurve(curvex,sigma,smoothX,X,XX,g,dg,d2g,isOpen);
	getdXcurve(curvey,sigma,smoothY,Y,YY,g,dg,d2g,isOpen);
	
	kappa.resize(curvex.size());
	for (int i=0; i<curvex.size(); i++) {
		// Mokhtarian 02' eqn (4)
		kappa[i] = (X[i]*YY[i] - XX[i]*Y[i]) / pow(X[i]*X[i] + Y[i]*Y[i], 1.5);
	}
}

/* find zero crossings on curvature */
vector<int> FindCSSInterestPoints(const vector<double>& kappa) {
	vector<int> crossings;
	for (int i=0; i<kappa.size()-1; i++) {
		if ((kappa[i] < 0 && kappa[i+1] > 0) || kappa[i] > 0 && kappa[i+1] < 0) {
			crossings.push_back(i);
		}
	}
	return crossings;
}

vector<int> EliminateCloseMaximas(const vector<int>& maximasv, map<int,double>& maximas) {
	//eliminate degenerate segments (of very small length)
	vector<int> maximasvv;
	for (int i=0;i<maximasv.size();i++) {
		if (i < maximasv.size()-1 && 
			maximasv[i+1] - maximasv[i] <= 4) 
		{ 
			//segment of small length (1 - 4) - eliminate one point, take largest sigma 
			if (maximas[maximasv[i]] > maximas[maximasv[i+1]]) {
				maximasvv.push_back(maximasv[i]);
			} else {
				maximasvv.push_back(maximasv[i+1]);
			}
			i++; //skip next element as well
		} else {
			maximasvv.push_back(maximasv[i]);
		}
	}
	return maximasvv;
}

/* compute the CSS image */
vector<int> ComputeCSSImageMaximas(const vector<double>& contourx_, const vector<double>& contoury_,
								   vector<double>& contourx, vector<double>& contoury,
								   bool isClosedCurve
								   ) 
{
	ResampleCurve(contourx_, contoury_, contourx, contoury, 200, !isClosedCurve);
	vector<Point2d> pl; PolyLineMerge(pl,contourx,contoury);
	
	map<int,double> maximas;
	
	Mat_<Vec3b> img(500,200,Vec3b(0,0,0)), contourimg(350,350,Vec3b(0,0,0));
	bool done = false;
//#pragma omp parallel for
	for (int i=0; i<490; i++) {
		if (!done) {
			double sigma = 1.0 + ((double)i)*0.1;
			vector<double> kappa, smoothx, smoothy;
			ComputeCurveCSS(contourx, contoury, kappa, smoothx, smoothy,sigma);
			
//			vector<vector<Point> > contours(1);
//			PolyLineMerge(contours[0], smoothx, smoothy);
//			contourimg = Vec3b(0,0,0);
//			drawContours(contourimg, contours, 0, Scalar(255,255,255), CV_FILLED);
			
			vector<int> crossings = FindCSSInterestPoints(kappa);
			if (crossings.size() > 0) {
				for (int c=0; c<crossings.size(); c++) {
					img(i,crossings[c]) = Vec3b(255,0,0);
//					circle(contourimg, contours[0][crossings[c]], 5, Scalar(0,0,255), CV_FILLED);
					
					if (c < crossings.size()-1) {
						if (fabs( (double)(crossings[c] - crossings[c+1])) < 5.0) {
							//this is a maxima
							int idx = (crossings[c] + crossings[c+1]) / 2;
//#pragma omp critical
							maximas[idx] = (maximas[idx] < sigma) ? sigma : maximas[idx];
							
							circle(img, Point(idx,i), 1, Scalar(0,0,255), CV_FILLED);
						}
					}
				}
//				char buf[128]; sprintf(buf, "evolution_%05d.png", i);
//				imwrite(buf, contourimg);
//				imshow("evolution", contourimg);
//				waitKey(30);
			} else {
				done = true;
			}

		}
	}
	
	//find largest sigma
	double max_sigma = 0.0;
	for (map<int,double>::iterator itr = maximas.begin(); itr!=maximas.end(); ++itr) {
		if (max_sigma < (*itr).second) {
			max_sigma = (*itr).second;
		}
	}
	//get segments with largest sigma
	vector<int> maximasv;
	for (map<int,double>::iterator itr = maximas.begin(); itr!=maximas.end(); ++itr) {
		if ((*itr).second > max_sigma/8.0) {
			maximasv.push_back((*itr).first);
		}
	}
	//eliminate degenerate segments (of very small length)
	vector<int> maximasvv = EliminateCloseMaximas(maximasv,maximas);	//1st pass
	maximasvv = EliminateCloseMaximas(maximasvv,maximas);				//2nd pass
	maximasv = maximasvv;
	for (vector<int>::iterator itr = maximasv.begin(); itr!=maximasv.end(); ++itr) {
		cout << *itr << " - " << maximas[*itr] << endl;
	}
//	Mat zoom; resize(img,zoom,Size(img.rows*2,img.cols*2));
	imshow("css image",img);
	waitKey();
	return maximasv;
}

#pragma mark Curve Matching

/* calculate the "centroid distance" for the curve */
void GetCurveSignature(const vector<Point2d>& a, vector<double>& signature) {
	signature.resize(a.size());
	Scalar a_mean = mean(a); Point2d a_mpt(a_mean[0],a_mean[1]);
	
	//centroid distance
	for (int i=0; i<a.size(); i++) {
		signature[i] = norm(a[i] - a_mpt);
	}
}

/* from http://paulbourke.net/miscellaneous/correlate/ */
double CalcCrossCorrelation(const vector<double>& x, const vector<double>& y) {
	assert(x.size()==y.size());
	int i,j,n = x.size();
	double mx,my,sx,sy,sxy,denom,r;
	
	/* Calculate the mean of the two series x[], y[] */
	mx = 0;
	my = 0;   
	for (i=0;i<n;i++) {
		mx += x[i];
		my += y[i];
	}
	mx /= n;
	my /= n;
	
	/* Calculate the denominator */
	sx = 0;
	sy = 0;
	for (i=0;i<n;i++) {
		sx += (x[i] - mx) * (x[i] - mx);
		sy += (y[i] - my) * (y[i] - my);
	}
	denom = sqrt(sx*sy);
	
	/* Calculate the correlation series */
//	for (delay=-maxdelay;delay<maxdelay;delay++) 
	int delay = 0;
	{
		sxy = 0;
		for (i=0;i<n;i++) {
			j = i + delay;
			if (j < 0 || j >= n)
				continue;
			else
				sxy += (x[i] - mx) * (y[j] - my);
			/* Or should it be (?)
			 if (j < 0 || j >= n)
			 sxy += (x[i] - mx) * (-my);
			 else
			 sxy += (x[i] - mx) * (y[j] - my);
			 */
		}
		r = sxy / denom;
		
		/* r is the correlation coefficient at "delay" */
	}
	return r;
}

/* calculate the similarity score between two curve segments
 Mai 2010, "Affine-invariant shape matching and recognition under partial occlusion", section 4.1
 */
double MatchTwoSegments(const vector<Point2d>& a_, const vector<Point2d>& b_) {
	assert(a_.size() == b_.size()); //cross correlation will work only for similar length curves
	if(a_.size() <= 1 || b_.size() <= 1) {
		cerr << "degenerate: a_.size() " << a_.size() << " b_.size() " << b_.size() << endl;
		return -1.0; //check degenrate case
	}
	
	vector<double> a_x(a_.size()),a_y(a_.size()),b_x(b_.size()),b_y(b_.size());
	vector<double> a_x_(a_.size()),a_y_(a_.size()),b_x_(b_.size()),b_y_(b_.size());
	vector<Point2d> a = a_, b = b_;
//	PolyLineSplit(a_, a_x_, a_y_); ResampleCurve(a_x_, a_y_, a_x, a_y, 50); PolyLineMerge(a, a_x, a_y);
//	PolyLineSplit(b_, b_x_, b_y_); ResampleCurve(b_x_, b_y_, b_x, b_y, 50); PolyLineMerge(b, b_x, b_y);
	
	Scalar a_mean = mean(a), b_mean = mean(b); 
	Point2d a_mpt(a_mean[0],a_mean[1]),b_mpt(b_mean[0],b_mean[1]);
	vector<Point2d> a_m(a.size()),b_m(b.size());
	for (int i=0; i<a.size(); i++) { a_m[i] = a[i] - a_mpt; }
	for (int i=0; i<b.size(); i++) { b_m[i] = b[i] - b_mpt; }
	
	Mat_<double> a_mM = Mat(a_m).reshape(1).t();
	Mat_<double> b_mM = Mat(b_m).reshape(1).t();
	SVD asvd(a_mM),bsvd(b_mM);
	vector<Point2d> a_canon(a.size()),b_canon(b.size());
	Mat(asvd.vt.t()).copyTo(a_mM); 
	a_mM.reshape(2).copyTo(Mat(a_canon));
	Mat(bsvd.vt.t()).copyTo(b_mM); 
	b_mM.reshape(2).copyTo(Mat(b_canon));

	
	vector<double> a_sig; GetCurveSignature(a_canon, a_sig);
	vector<double> b_sig; GetCurveSignature(b_canon, b_sig);

	double cc = CalcCrossCorrelation(a_sig, b_sig);

#if 0
#ifdef HAVE_MATHGL
	{
		mglGraph gr;
		gr.SubPlot(2, 1, 0, "");
	
		vector<double> a_canon_x,a_canon_y;
		PolyLineSplit(a_canon, a_canon_x, a_canon_y);
		vector<double> b_canon_x,b_canon_y;
		PolyLineSplit(b_canon, b_canon_x, b_canon_y);
		
		mglData mgl_a_x(&(a_canon_x[0]),a_canon_x.size()),mgl_a_y(&(a_canon_y[0]),a_canon_y.size());
		mglData mgl_b_x(&(b_canon_x[0]),b_canon_x.size()),mgl_b_y(&(b_canon_y[0]),b_canon_y.size());
		
		gr.Title("Canonical");
		gr.Aspect(1, 1);	
		gr.SetRanges(-.5, .5, -.5, .5);
		gr.Axis(); 
		gr.Grid();
		gr.Plot(mgl_a_x,mgl_a_y);
		gr.Plot(mgl_b_x,mgl_b_y);	

		
		gr.SubPlot(2, 1, 1, "");
		mglData x(&(a_sig[0]),a_sig.size()),x1(&(b_sig[0]),b_sig.size());
		
		gr.Title("Signature");
		gr.SetRanges(0, max(a_sig.size(),b_sig.size()), 0, 0.55);
		gr.Axis(); 
		gr.Grid();
		gr.Plot(x);
		gr.Plot(x1);

		Mat img(gr.GetHeight(),gr.GetWidth(),CV_8UC3,(void*)gr.GetRGB());
		stringstream ss; ss << "cross correlation " << cc;
		putText(img, ss.str(), Point(10,20), CV_FONT_NORMAL, 1.0, Scalar(255), 2);
		imshow("tmp", img);
		waitKey();
	}	
#endif
#endif
	
	return cc; // > 0.8 ? cc : 0.0;
}

/* match the two curves using adapted Smith-Waterman aligning algorithm
 Mai 2010, "Affine-invariant shape matching and recognition under partial occlusion", section 4.2 */ 

Mat_<double> GetSmithWatermanHMatrix(const vector<vector<Point2d> >& a, const vector<vector<Point2d> >& b) {
	int M = a.size();
	int N = b.size();

	//Smith-Waterman
	Mat_<double> H(M,N-1,0.0);
	for (int i=1; i<M; i++) {
		for (int j=1; j<N-1; j++) {
			vector<double> v(4,0.0); 
			v[1] = H(i-1,j-1) + MatchTwoSegments(a[i], b[j]);
			v[2] = H(i-1,j) - 1.0;
			v[3] = H(i,j-1) - 1.0;
			H(i,j) = *(max_element(v.begin(), v.end()));
		}
	}
	cout << H << endl;
	return H;
}	

/* original Smith Waterman algorithm */
double MatchCurvesSmithWaterman(const vector<vector<Point2d> >& a, const vector<vector<Point2d> >& b, vector<Point>& traceback) 
{	
	Mat_<double> H = GetSmithWatermanHMatrix(a,b);
	Point maxp; double maxval;
	minMaxLoc(H, NULL, &maxval, NULL, &maxp);
	while (H(maxp.y,maxp.x) != 0) {
//				cout << "H(maxp.y-1,maxp.x-1) > H(maxp.y,maxp.x-1)" << H(maxp.y-1,maxp.x-1) << " > " << H(maxp.y,maxp.x-1) << endl;
		if (H(maxp.y-1,maxp.x-1) > H(maxp.y,maxp.x-1) &&
			H(maxp.y-1,maxp.x-1) > H(maxp.y-1,maxp.x)) 
		{
			maxp = maxp - Point(1,1);
			traceback.push_back(maxp);
		} else
		if (H(maxp.y-1,maxp.x) > H(maxp.y-1,maxp.x-1) &&
			H(maxp.y-1,maxp.x) > H(maxp.y,maxp.x-1)) 
		{
			maxp.y--;
			traceback.push_back(maxp);
		} else
		if (H(maxp.y,maxp.x-1) > H(maxp.y-1,maxp.x-1) &&
			H(maxp.y,maxp.x-1) > H(maxp.y-1,maxp.x)) 
		{
			maxp.x--;
			traceback.push_back(maxp);
		}
		else {
			break;
		}
	}
	for (int k=0; k<traceback.size(); k++) {
		cout << traceback[k] << " -> ";
	} 
	cout << endl;
	return maxval;
}

/* adapted Smith Waterman */
double AdaptedMatchCurvesSmithWaterman(const vector<vector<Point2d> >& a, const vector<vector<Point2d> >& b, vector<Point>& traceback) 
{
	int M = a.size();
	int N = b.size();
	
	Mat_<double> H = GetSmithWatermanHMatrix(a,b);

	vector<vector<Point> > tracebacks;
	vector<Point> max_traceback;
	int max_traceback_len = 0;
	for (int i=M-1; i>=2; i--) {
		for (int j=N-2; j>=2; j--) {
			if (i < max_traceback_len || j < max_traceback_len) {
				continue; //skip it, it already can't be longer..
			}
			
			//Traceback
			vector<Point> tmp_traceback;
			Point maxp = Point(i,j);
			tmp_traceback.push_back(maxp);
//			maxp = maxp - Point(1,1);
//			tmp_traceback.push_back(maxp);
			bool movedup = false,movedleft = false;
			while (H(maxp.y,maxp.x) != 0 && maxp.y > 1 && maxp.x > 1) {
				if (H(maxp.y-1,maxp.x-1) > H(maxp.y,maxp.x-1) &&
					H(maxp.y-1,maxp.x-1) > H(maxp.y-1,maxp.x)) 
				{
//					cout << "move left-up" << endl;
					maxp = maxp - Point(1,1);
					traceback.push_back(maxp);
				} else if (H(maxp.y-1,maxp.x) > H(maxp.y-1,maxp.x-1) &&
						H(maxp.y-1,maxp.x) > H(maxp.y,maxp.x-1)) 
				{
//					cout << "move up" << endl;
					maxp.y--;
					movedup = true;
				} else if (H(maxp.y,maxp.x-1) > H(maxp.y-1,maxp.x-1) &&
						H(maxp.y,maxp.x-1) > H(maxp.y-1,maxp.x)) 
				{
//					cout << "move left" << endl;
					maxp.x--;
					movedleft = true;
				}
				if (movedup && movedleft) {
					traceback.push_back(maxp);
					movedup = movedleft = false;
				}
			}
			for (int k=0; k<tmp_traceback.size(); k++) {
				cout << tmp_traceback[k] << " -> ";
			}
			cout << endl;
			if (tmp_traceback.size() > max_traceback_len || 
				(
				 tmp_traceback.size() == max_traceback_len && //if equal - look for highest match
				 H(tmp_traceback.front().y,tmp_traceback.front().x) > H(max_traceback.front().y,max_traceback.front().x)
				)
				) 
			{
				max_traceback_len = tmp_traceback.size();
				max_traceback = tmp_traceback;
				cout << "taking traceback of length " << max_traceback_len << endl;
			}
		}
	}
	
	traceback = max_traceback;
	
	return H(traceback[0].y,traceback[0].x);
}








//#include "atfz.h"
//#include <math.h>
//#include <vector>
//
//atfx::atfx(void)
//{
//}
//
//
//atfx::~atfx(void)
//{
//}
//
//
//
//void atfx::setInputSketch(cv::Mat image)
//{
//	this->sketch = image.clone();
//	this->step_image = image.clone();
//}
//
//cv::Mat atfx::applyBinarization() {
//
//	cv::imshow("step_image",step_image);
//	cv::waitKey(3000);
//	/*
//	cv::Size size = this->step_image.size();
//	cv::Mat rgb = cv::Mat::zeros(size, step_image.type());
//
//
//	for (int i = 0; i <  this->step_image.rows  ; i++) {
//		for (int j = 0; j <  this->step_image.cols ; j++) {
//			rgb.at<int>(i, j)= this->step_image.at<int>(i, j);
//		}
//	}
//
//	cv::Mat gray =cv::Mat::zeros(size, CV_8U ); //opencv_core.cvCreateImage(size, 8, 1);
//	//  opencv_imgproc.cvCvtColor(rgb, gray, 7);
//	cv::cvtColor(rgb, gray,cv::COLOR_RGB2GRAY, 1);
//
//
//	this->binarizedImage =  cv::Mat();// opencv_core.cvCreateImage(size, 8, 1);
//
//	cv::threshold(gray, this->binarizedImage, 0.0, 255.0, 8);
//	// opencv_imgproc.cvThreshold(gray, this->binarizedImage, 0.0, 255.0, 8);
//
//	// opencv_core.cvReleaseImage(rgb);
//	// opencv_core.cvReleaseImage(gray);
//	*/
//	//cv::threshold(step_image, this->binarizedImage, 0.0, 255.0, 8);
//	//this->step_image = this->binarizedImage;
//
//	//cv::imshow("step_image",step_image);
//	//cv::waitKey(0);
//
//	return this->step_image;//  this->binarizedImage;
//}
//
//cv::Mat atfx::applyGaussian(double sigma)
//{
//	this->gaussianImage =  cv::Mat();// opencv_core.cvCreateImage(opencv_core.cvGetSize(this->step_image), this->step_image.depth(), this->step_image.nChannels());
//	double sigma_from_width = 0.3 * (sigma * 0.5  - 1.0) + 0.4+0.5;
//	cv::Size ksize=   cv::Size( sigma_from_width, sigma_from_width );
//	cv::blur(this->step_image, this->gaussianImage, ksize) ;//.cvSmooth(this->step_image, this->gaussianImage, 2, 0, 0, sigma_from_width, sigma_from_width);
//	this->step_image = this->gaussianImage;
//	return this->gaussianImage;
//}
//
//cv::Mat atfx::applyThinning()
//{
//	this->thinnedImage = this->step_image;
//
//	atfz alg ;//= new atfz();
//	alg.setImage(this->thinnedImage);
//
//
//
//	this->step_image = alg.getThinnedImage();
//	return this->step_image;
//}
//
//int atfx::calculateRequiredThinningIterations(cv::Mat image) {
//	
//	 
//	cv::Mat  mat_image = image.clone();
//	cv::Mat mat_image_without_borders=image.clone();
//	
// 
//	for (int i = 0; i < image.cols; i++) {
//		for (int j = 0; j < image.rows; j++)
//		{
//			/*if (image.at<int>( j, i) <1)
//				mat_image.at<int>[i][j] = 1;
//			else {
//				mat_image[i][j] = 0;
//			}
//*/
//			mat_image_without_borders.at<int>( j, i) = 0;
//		}
//
//	}
//
//	int removing_iterations = 0;
//	bool more_blacks;
//	do { 
//		
//		more_blacks = false;
//
//		for (int i = 1; i < image.cols - 1; i++) {
//			for (int j = 1; j < image.rows - 1; j++) {
//				if (mat_image.at<int>( j, i) == 1) {
//					more_blacks = true;
//
//					bool is_border_pixel = false;
//					for (int x = i - 1; x <= i + 1; x++) {
//						for (int y = j - 1; y <= j + 1; y++) {
//							if (mat_image.at<int>( y, x) == 0) {
//								is_border_pixel = true;
//							}
//						}
//					}
//
//					if (is_border_pixel)
//						mat_image_without_borders.at<int>( j, i)  = 0;
//					else
//						mat_image_without_borders.at<int>( j, i) = 1;
//				}
//				else {
//					mat_image_without_borders.at<int>( j, i) = 0;
//				}
//			}
//		}
//
//		for (int i = 1; i < image.cols - 1; i++) {
//			for (int j = 1; j < image.rows - 1; j++) {
//				mat_image.at<int>( j, i) = mat_image_without_borders.at<int>( j, i);
//			}
//		}
//	
//		removing_iterations++;
//	
//
//	}
//	while (more_blacks);
//
//	return removing_iterations * 2 + 1;
//}
//
//cv::Point atfx::pointClockwiseIndex(int index) {
//	switch (index) {
//	case 0:
//		return  cv::Point(0, 0);
//	case 1:
//		return  cv::Point(-1, 0);
//	case 2:
//		return  cv::Point(-1, 1);
//	case 3:
//		return  cv::Point(0, 1);
//	case 4:
//		return  cv::Point(1, 1);
//	case 5:
//		return  cv::Point(1, 0);
//	case 6:
//		return  cv::Point(1, -1);
//	case 7:
//		return  cv::Point(0, -1);
//	case 8:
//		return  cv::Point(-1, -1);
//	}
//
//	return  cv::Point(-2, -2);
//}
//
//int atfx::blackToWhiteTransitionCount(cv::Mat image, cv::Point p) {
//	short counter = 0;
//	for (int index = 1; index < 9; index++) {
//		cv::Point first_point = pointClockwiseIndex(index);
//		bool first_condition =( image.at<int>((int)(p.x + first_point.x)  ,(int)( p.y + first_point.y))  == 0);
//		cv::Point second_point;
//		//Point second_point;
//		if (index + 1 == 9)
//			second_point = pointClockwiseIndex(1);
//		else {
//			second_point = pointClockwiseIndex(index + 1);
//		}
//		bool second_condition =( image.at<int> ((int)( p.x + second_point.x) , (int)( p.y + second_point.y))  == 1) ;
//
//		if ((first_condition) && (second_condition)) {
//			counter = (short)(counter + 1);
//		}
//	}
//
//	return counter;
//}
//
//double atfx::getSensitivityMeasurement(cv::Mat sketch, cv::Mat skeleton) {
//	double SM = 0.0;
//	double total_black_pixel_count = 1.0;
//
//	for (int i = 1; i < skeleton.rows - 1; i++) {
//		for (int j = 1; j < skeleton.cols - 1; j++) {
//			if (skeleton.at<int>(i, j) == 0) {
//				total_black_pixel_count += 1.0;
//				if ((blackToWhiteTransitionCount(skeleton,   cv::Point(i, j)) > 2) || (sketch.at<int>( i, j)  != 0.0) || (blackToWhiteTransitionCount(skeleton,   cv::Point(i, j)) == 0)) {
//					SM += 1.0;
//				}
//			}
//		}
//	}
//
//	double ratio = SM / total_black_pixel_count;
//
//	double toReturn = floor(ratio) +  floor(ratio * 1000.0) / 1000.0;
//
//	return toReturn;
//}
//
//cv::Mat atfx::getATFImage()
//{
//	int estimated_width = calculateRequiredThinningIterations(this->sketch);
//
//	//  List image_list = new ArrayList();
//	std::vector<  cv::Mat>  image_list;
//
//
//	//List SM_list = new ArrayList();
//	std::vector<  double>  SM_list;
//	applyBinarization();
//
//	applyThinning();
//	SM_list.push_back( getSensitivityMeasurement(this->sketch, this->step_image) );
//
//	image_list.push_back(this->step_image);
//
//
//
//	this->step_image = this->sketch;
//
//	cv::imshow("step_imageZs", this->step_image);  cv::waitKey(1000);
//
//
//	//List sigma_list = new ArrayList();
//	std::vector<  int > sigma_list;
//
//	sigma_list.push_back( 0 );
//
//
//
//	for (int index = 1; index <= estimated_width; index++)
//	{
//		int width = index * 2 + 1;
//		int i = index;
//
//		 cv::imshow("step_imageZ1", this->step_image);  cv::waitKey(1000);
//
//		applyGaussian(width);
//
//		 cv::imshow("step_imageZ2", this->step_image);  cv::waitKey(1000);
//
//		sigma_list.push_back( i );
//
//		applyBinarization();
// cv::imshow("step_imageZ3", this->step_image);  cv::waitKey(1000);
//
//		applyThinning();
// cv::imshow("step_imageZ4", this->step_image);  cv::waitKey(1000);
//
//		SM_list.push_back( getSensitivityMeasurement(this->sketch, this->step_image) );
//
//		image_list.push_back(this->step_image);
//
//		  cv::imshow("step_imageZ5", this->step_image);  cv::waitKey(1000);
//
//		//if (this->gaussianImage != null) {
//		//  // opencv_core.cvReleaseImage(this->gaussianImage);
//		//}
//
//		this->step_image = this->sketch;
//	}
//
//	int SM_index_min = 0;
//	int N = SM_list.size();
//	// for (Iterator i$ = sigma_list.iterator(); i$.hasNext(); )
//	for(int i=0;i< sigma_list.size();i++)
//	{
//		int sigma_index =i;// ((Integer)i$.next()).intValue();
//		if ( (double)SM_list[sigma_index]    <  (double)SM_list[SM_index_min])
//		{
//			SM_index_min = sigma_index;
//		}
//	}
//
//	for (int index = 0; index < N; index++) {
//		if (index != SM_index_min) {
//			cv::Mat image_to_release = (cv::Mat)image_list[index];
//			// opencv_core.cvReleaseImage(image_to_release);
//		}
//	}
//
//	cv::Mat image = (cv::Mat)image_list[SM_index_min];
//	return image;
//}