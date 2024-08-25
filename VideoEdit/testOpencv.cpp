#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>

int testOpencv() {
	// 文件路径
	std::string backgroundImagePath = "C:/Users/xctan/Pictures/2.jpg";
	std::string smallImagePath = "C:/Users/xctan/Pictures/4.jpg";

	// 读取大背景图片和小图片
	cv::Mat background = cv::imread(backgroundImagePath);
	cv::Mat smallImage = cv::imread(smallImagePath, cv::IMREAD_UNCHANGED);

	// 检查图片是否读取成功
	if (background.empty() || smallImage.empty()) {
		std::cerr << "Error: Could not load images." << std::endl;
		return -1;
	}

	// 获取小图片的尺寸
	int smallWidth = smallImage.cols;
	int smallHeight = smallImage.rows;

	// 确定小图片的放置位置 (例如，将其放置在大背景的中心)
	int xOffset = (background.cols - smallWidth) / 2;
	int yOffset = (background.rows - smallHeight) / 2;
	xOffset = background.cols - smallImage.cols;
	yOffset = 0;

	// 检查是否有 alpha 通道并叠加
	if (smallImage.channels() == 4) {  // 如果小图片有 alpha 通道
		cv::Mat overlay;
		background(cv::Rect(xOffset, yOffset, smallWidth, smallHeight)).copyTo(overlay);

		for (int y = 0; y < smallHeight; ++y) {
			for (int x = 0; x < smallWidth; ++x) {
				// 获取 alpha 通道的值
				double alpha = smallImage.at<cv::Vec4b>(y, x)[3] / 255.0;
				// 混合颜色通道
				for (int c = 0; c < 3; ++c) {
					overlay.at<cv::Vec3b>(y, x)[c] =
						static_cast<uchar>(alpha * smallImage.at<cv::Vec4b>(y, x)[c] +
							(1 - alpha) * overlay.at<cv::Vec3b>(y, x)[c]);
				}
			}
		}

		// 将叠加后的区域放回背景
		overlay.copyTo(background(cv::Rect(xOffset, yOffset, smallWidth, smallHeight)));
	}
	else {
		// 如果小图片没有 alpha 通道，直接复制
		cv::Mat resizeSmallMat;
		cv::resize(smallImage, resizeSmallMat, cv::Size(smallImage.cols, smallImage.rows * 2), 0, 0, cv::INTER_LINEAR);
		resizeSmallMat.copyTo(background(cv::Rect(xOffset, yOffset, smallWidth, smallHeight*2)));
	}

	// 显示结果
	cv::imshow("Combined Image", background);
	cv::waitKey(0);

	// 保存结果
	//cv::imwrite("path/to/your/combined_image.jpg", background);  // 保存叠加后的图片

	return 0;
}
