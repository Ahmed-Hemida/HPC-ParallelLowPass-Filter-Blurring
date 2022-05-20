#include <iostream>
#include <math.h>
#include <stdlib.h>
#include<string.h>
#include<msclr\marshal_cppstd.h>
#include <ctime>// include this header 
#pragma once
#include <mpi.h>
#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

using namespace std;
using namespace msclr::interop;

int* inputImage(int* w, int* h, System::String^ imagePath) //put the size of image in w & h
{
	int* input;


	int OriginalImageWidth, OriginalImageHeight;

	//*********************************************************Read Image and save it to local arrayss*************************	
	//Read Image and save it to local arrayss

	System::Drawing::Bitmap BM(imagePath);

	OriginalImageWidth = BM.Width;
	OriginalImageHeight = BM.Height;
	*w = BM.Width;
	*h = BM.Height;
	int *Red = new int[BM.Height * BM.Width];
	int *Green = new int[BM.Height * BM.Width];
	int *Blue = new int[BM.Height * BM.Width];
	input = new int[BM.Height*BM.Width];
	for (int i = 0; i < BM.Height; i++)
	{
		for (int j = 0; j < BM.Width; j++)
		{
			System::Drawing::Color c = BM.GetPixel(j, i);

			Red[i * BM.Width + j] = c.R;
			Blue[i * BM.Width + j] = c.B;
			Green[i * BM.Width + j] = c.G;

			input[i*BM.Width + j] = ((c.R + c.B + c.G) / 3); //gray scale value equals the average of RGB values

		}

	}
	return input;
}


void createImage(int* image, int width, int height, int index)
{
	System::Drawing::Bitmap MyNewImage(width, height);


	for (int i = 0; i < MyNewImage.Height; i++)
	{
		for (int j = 0; j < MyNewImage.Width; j++)
		{
			//i * OriginalImageWidth + j
			if (image[i*width + j] < 0)
			{
				image[i*width + j] = 0;
			}
			if (image[i*width + j] > 255)
			{
				image[i*width + j] = 255;
			}
			System::Drawing::Color c = System::Drawing::Color::FromArgb(image[i*MyNewImage.Width + j], image[i*MyNewImage.Width + j], image[i*MyNewImage.Width + j]);
			MyNewImage.SetPixel(j, i, c);
		}
	}
	MyNewImage.Save("..//Data//Output//outputRes" + index + ".png");
	cout << "result Image Saved " << index << endl;
}


int main()
{
	int ImageWidth = 4, ImageHeight = 4;

	int start_s, stop_s, TotalTime = 0;

	System::String^ imagePath;
	std::string img;
	img = "path// img //to// load";
	imagePath = marshal_as<System::String^>(img);
	int* imageData = inputImage(&ImageWidth, &ImageHeight, imagePath);


	start_s = clock();

	int index = 13;

	MPI_Init(NULL, NULL);


	int rank, size;


	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);


	
	int localhight;
	int localArrSize;
	int* newImg=new int[ImageWidth* ImageHeight];
		localhight = ImageHeight / size;
		localArrSize = localhight * ImageWidth;

		int* Localarr=new int[localArrSize];
	MPI_Scatter(imageData, localArrSize, MPI_INT, Localarr, localArrSize, MPI_INT, 1, MPI_COMM_WORLD);
	
	int sum, column, row;
	int* localNewArr = new int[localhight* ImageWidth];
	for (int c = 0; c < localhight; c++)
	{
		column = ImageWidth * c;

		for (int r = 0; r < ImageWidth; r++)
		{
			row = column + r;
			sum = 0;
			for (int i = column; i < (column + 3); i++)
			{
				for (int j = row; j < (row + 3); j++)
				{
					sum += Localarr[i + (j % ImageWidth)];

				}
			}
		
			sum /= (9);
			if (sum < 0)
				sum = 0;
			if (sum > 255)
				sum = 255;

			//if(column + row >=10000)
			//	cout << "rank"<< rank << " , Sum :" << sum << " , column :" << column << " , row :" << row << ", column + row :" << column + row << " , localhight:" << localhight << ", :ImageWidthendl " << ImageWidth << endl;

			localNewArr[ row] = sum;

		}
	}
	MPI_Gather(localNewArr, localArrSize, MPI_INT, newImg, localArrSize, MPI_INT, 1, MPI_COMM_WORLD);
	if (rank == 1)
	{
		System::Drawing::Bitmap MyNewImage(ImageWidth, ImageHeight);
		int sum, column, row;

		for (int c = 0; c < ImageHeight; c++)
		{
			column = ImageWidth * c;
			for (int r = 0; r < ImageWidth; r++)
			{
	
				row = column + r;
				//cout << "rank" << rank << " , column :" << column << " , row :" << row << ", column + row :" << column + row << " , ImageHeight :" << ImageHeight << ", :ImageWidthendl " << ImageWidth << endl;
				sum = newImg[row];
				if (sum < 0)
					sum = 0;
				if (sum > 255)
					sum = 255;
				System::Drawing::Color color = System::Drawing::Color::FromArgb(sum, sum, sum);
				MyNewImage.SetPixel(r, c, color);
			}
		}
		MyNewImage.Save("..//path to //Data//Output//outputRes);
		cout << "result Image Saved " << index << endl;
		stop_s = clock();
		TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
		cout << "time: " << TotalTime << endl;

		free(imageData);
		system("pause");

	}


	//msiexec -n 3 name of the project.exe

	MPI_Finalize();

	return 0;
}



