#pragma once
#include"triangle.h"
#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<time.h>
#include<stdio.h>
#include<Qimage>
#include<Qstring>
constexpr double M_PI = 3.1415926;
double DEG2RAD(double deg);
struct object
{
public:
	virtual void Init()=0;
	virtual void Update()=0;
	void set_model();
	void set_trans(vector_3f& trans_delta);
	void set_rotat(vector_3f& rotat_delta);
public:
	matrix_4f Model;
	vector_3f rotat;
	vector_3f trans;
};
