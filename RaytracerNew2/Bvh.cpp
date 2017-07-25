#include "Bvh.h"

#include "Logger.h"
#include "Tools.h"

#include <thread>
#include <ppl.h>

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Bvh::Bvh()
:myPrimitivesPerNode(2)
,mySplitHeuristic(SAH)
,INTERSECTION_COST(1.)
,TRAVERSAL_COST(1.)
,myNumberOfNodes(0)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Bvh::Bvh(std::vector<IPrimitive::ptr>& _primitives) 
:myPrimitives(_primitives) 
,myPrimitivesPerNode(2) 
, mySplitHeuristic(SAH)
,INTERSECTION_COST(1.)
,TRAVERSAL_COST(1.)
,myNumberOfNodes(0)
{ 
	build(); 
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Bvh::~Bvh()
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void Bvh::build()
{
	std::vector<BvhData, Eigen::aligned_allocator<BvhData>> myData;
	myData.reserve(myPrimitives.size());

	for (unsigned int i = 0; i < myPrimitives.size(); i++)
	{
		BoundingBox box = myPrimitives[i]->getWorldBoundingBox();
		myData.push_back(BvhData(i, box, box.getCentroid()));
	}

	BoundingBox box;

	for (unsigned int i = 0; i < myPrimitives.size(); i++)
	{
		box = BoundingBox::unionBox(box, myPrimitives[i]->getWorldBoundingBox());
	}

	root = BvhNode::ptr(new BvhNode);
	root->myBoundingBox = box;
	ILogger::log(ILogger::ALL) << "Bounding box " << box << "\n";

	buildBinNode(0, myPrimitives.size(), root, myData, 0);

	myNodes.resize(myNumberOfNodes);

	int num = 0;
	flatten(*root, &num);//0);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void Bvh::buildNode(int startIndex, int endIndex, BvhNode::ptr currentNode, const std::vector<BvhData, Eigen::aligned_allocator<BvhData>>& data, int numberNode)
{
	myNumberOfNodes++;

	int number = endIndex - startIndex;
	std::vector<double> surfaces(number);

	int indexSplit = -1;

	switch (mySplitHeuristic)
	{
	case MIDDLE_POINT:
	{
		if (number < myPrimitivesPerNode)
		{
			currentNode->myFirstPrimIndex = startIndex;
			currentNode->myNumberPrimitives = number;
		}
		else
		{
			Axis axis = getLargestAxisExtent(currentNode->myBoundingBox);
			double mid = midPoint(axis, currentNode->myBoundingBox);

			//predicat
			sortAxis compare(axis);
			std::sort(myPrimitives.begin() + startIndex, myPrimitives.begin() + endIndex, compare);

			for (auto i = startIndex; i < endIndex; i++)
			{
				if (myPrimitives[i]->getWorldBoundingBox().getCentroid()((int)axis) > mid)
				{
					indexSplit = i;
					break;
				}
			}
			if (indexSplit <= 0)
			{
				//median split
				indexSplit = (startIndex + endIndex) / 2;
			}
			ILogger::log(ILogger::ALL) << startIndex << " " << indexSplit << " " << endIndex << " " << numberNode << "\n";

			BoundingBox leftBBox;
			for (auto i = startIndex; i < indexSplit; i++)
			{
				leftBBox = BoundingBox::unionBox(leftBBox, myPrimitives[i]->getWorldBoundingBox());
			}

			BoundingBox rightBBox;
			for (auto i = indexSplit; i < endIndex; i++)
			{
				rightBBox = BoundingBox::unionBox(rightBBox, myPrimitives[i]->getWorldBoundingBox());
			}

			BvhNode::ptr left = BvhNode::ptr(new BvhNode);
			left->myBoundingBox = leftBBox;
			BvhNode::ptr right = BvhNode::ptr(new BvhNode);
			right->myBoundingBox = rightBBox;

			currentNode->addChildren(left, right);

			buildNode(startIndex, indexSplit, left, data, numberNode + 1);
			buildNode(indexSplit, endIndex, right, data, numberNode + 1);
		}
	}
	break;
	case MEDIAN:
	{
		Axis axis = getLargestAxisExtent(currentNode->myBoundingBox);
		double mid = midPoint(axis, currentNode->myBoundingBox);

		//predicat
		sortAxis compare(axis);
		std::sort(myPrimitives.begin() + startIndex, myPrimitives.begin() + endIndex, compare);

		indexSplit = (startIndex + endIndex) / 2;

		ILogger::log(ILogger::ALL) << startIndex << " " << indexSplit << " " << endIndex << " " << numberNode << "\n";

		BvhNode::ptr left = BvhNode::ptr(new BvhNode);
		BvhNode::ptr right = BvhNode::ptr(new BvhNode);

		currentNode->addChildren(left, right);

		buildNode(startIndex, indexSplit, left, data, numberNode + 1);
		buildNode(indexSplit, endIndex, right, data, numberNode + 1);

		break;
	}
	case SAH:
	default:
		double minCost = INTERSECTION_COST * number;
		int bestIndex = -1;
		int bestAxis = -1;

		//For each axis
		for (int axis = 0; axis < 3; axis++)
		{
			sortAxis compare((Axis)axis);
			std::sort(myPrimitives.begin() + startIndex, myPrimitives.begin() + endIndex, compare);

			BoundingBox box;
			for (auto i = startIndex, cpt = 0; i < endIndex; i++, cpt++)
			{
				box = BoundingBox::unionBox(box, myPrimitives[i]->getWorldBoundingBox());
				surfaces[cpt] = box.getSurfaceValue();
			}

			if (axis == 0)
				currentNode->myBoundingBox = box;

			double factor = INTERSECTION_COST / currentNode->myBoundingBox.getSurfaceValue();
			BoundingBox rightBox;
			for (auto j = number - 1; j >= 1; j--)
			{
				int index = startIndex + j;
				rightBox = BoundingBox::unionBox(rightBox, myPrimitives[index]->getWorldBoundingBox());
				double leftArea = surfaces[j - 1];
				double rightArea = rightBox.getSurfaceValue();

				double leftNumber = j;
				double rightNumber = number - j;

				double cost = 2 * TRAVERSAL_COST + factor * (leftNumber * leftArea + rightNumber * rightArea);

				if (cost < minCost)
				{
					minCost = cost;
					bestIndex = index;
					bestAxis = axis;
				}
			}
		} //end for axis

		if (bestIndex == -1)
		{
			currentNode->myFirstPrimIndex = startIndex;
			currentNode->myNumberPrimitives = number;
		}
		else
		{
			sortAxis compare((Axis)bestAxis);
			std::sort(myPrimitives.begin() + startIndex, myPrimitives.begin() + endIndex, compare);

			//if (number < 4)//myPrimitivesPerNode)
			{
				//currentNode->myFirstPrimIndex = startIndex;
				//currentNode->myNumberPrimitives = number;
			}
			//else
			{
				//std::cout << startIndex << " " << bestIndex << " " << endIndex << " " << numberNode << std::endl;

				BvhNode::ptr left = BvhNode::ptr(new BvhNode);
				BvhNode::ptr right = BvhNode::ptr(new BvhNode);

				currentNode->addChildren(left, right);

				buildNode(startIndex, bestIndex, left, data, numberNode + 1);
				buildNode(bestIndex, endIndex, right, data, numberNode + 1);
			}

		}
	}


		//	/* Vertex indices of the triangle */
		//	uint32_t idx0 = F(0, f), idx1 = F(1, f), idx2 = F(2, f);

		//	Point3f p0 = V.col(idx0), p1 = V.col(idx1), p2 = V.col(idx2);

		//	/* Compute the intersection positon accurately
		//	using barycentric coordinates */
		//	its.p = bary.x() * p0 + bary.y() * p1 + bary.z() * p2;

		//	/* Compute proper texture coordinates if provided by the mesh */
		//	if (UV.size() > 0)
		//		its.uv = bary.x() * UV.col(idx0) +
		//		bary.y() * UV.col(idx1) +
		//		bary.z() * UV.col(idx2);

		//	/* Compute the geometry frame */
		//	its.geoFrame = Frame((p1 - p0).cross(p2 - p0).normalized());

		//	if (N.size() > 0) {
		//		/* Compute the shading frame. Note that for simplicity,
		//		the current implementation doesn't attempt to provide
		//		tangents that are continuous across the surface. That
		//		means that this code will need to be modified to be able
		//		use anisotropic BRDFs, which need tangent continuity */

		//		its.shFrame = Frame(
		//			(bary.x() * N.col(idx0) +
		//				bary.y() * N.col(idx1) +
		//				bary.z() * N.col(idx2)).normalized());
		//	}
		//	else {
		//		its.shFrame = its.geoFrame;
		//	}
		//}

		//return foundIntersection;
}

//void Bvh::buildNode(int startIndex, int endIndex, BvhNode * currentNode, const std::vector<BvhData>& data, int numberNode)
//{
//	//BoundingBox box;
//	//for (auto i = startIndex; i < endIndex; i++)
//	//{
//	//	box = BoundingBox::unionBox(box, primitives[i]->getWorldBoundingBox());
//	//}
//
//	int number = endIndex - startIndex;
//	if (number <= myPrimitivesPerNode)
//	{
//		currentNode->myFirstPrimIndex = startIndex;
//		currentNode->myNumberPrimitives = number;
//	}
//	else
//	{
//		Axis axis = getLargestAxisExtent(currentNode->myBoundingBox);
//		double mid = midPoint(axis, currentNode->myBoundingBox);
//		int indexSplit = -1;
//
//		//predicat
//		sortAxis compare(axis);
//		std::sort(myPrimitives.begin() + startIndex, myPrimitives.begin() + endIndex, compare);
//
//		for (auto i = startIndex; i < endIndex; i++)
//		{
//			if (myPrimitives[i]->getWorldBoundingBox().getCentroid()((int)axis) > mid)
//			{
//				indexSplit = i;
//				break;
//			}
//		}
//		if (indexSplit <= 0)
//		{
//			indexSplit = (startIndex + endIndex) / 2;
//		}
//
//		std::cout << startIndex << " " << indexSplit << " " << endIndex << " " << numberNode << std::endl;
//
//		BoundingBox leftBBox;
//		for (auto i = startIndex; i < indexSplit; i++)
//		{
//			leftBBox = BoundingBox::unionBox(leftBBox, myPrimitives[i]->getWorldBoundingBox());
//		}
//
//		BoundingBox rightBBox;
//		for (auto i = indexSplit; i < endIndex; i++)
//		{
//			rightBBox = BoundingBox::unionBox(rightBBox, myPrimitives[i]->getWorldBoundingBox());
//		}
//
//		BvhNode* left = new BvhNode;
//		left->myBoundingBox = leftBBox;
//		BvhNode* right = new BvhNode;
//		right->myBoundingBox = rightBBox;
//
//		currentNode->addChildren(left, right);
//
//		buildNode(startIndex, indexSplit, left, data, numberNode + 1);
//		buildNode(indexSplit, endIndex, right, data, numberNode + 1);
//
//
//		//	std::vector<Object::ptr> left;
//		//	std::vector<Object::ptr> right;
//
//		//	for (auto i = 0; i < primitives.size(); i++)
//		//	{
//		//		if (primitives[i]->getWorldBoundingBox().getCentroid()((int)axis) < mid)
//		//		{
//		//			left.push_back(primitives[i]);
//		//		}
//		//		else
//		//		{
//		//			right.assign(primitives.begin() + i, primitives.end());
//		//		}
//		//	}
//		//}
//
//
//
//	}
//}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
Bvh::Axis Bvh::getLargestAxisExtent(const BoundingBox& box)
{
	Vector3d dist = box.getMax() - box.getMin();

	if (dist.x() > dist.y() && dist.x() > dist.z())
		return Axis::X_AXIS;
	else if (dist.z() > dist.x() && dist.z() > dist.y())
		return Axis::Z_AXIS;
	return Y_AXIS;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
double Bvh::midPoint(Axis axis, const BoundingBox & box)
{
	return box.getCentroid()((int) axis);
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
bool Bvh::intersection(const Ray & _ray, Intersection& inter, bool shadowRay)
{
	bool shouldCompute = inter.computeIntersect;
	if (!shouldCompute) //Not really useful, but...
	{
		//inter.t = tools::MAX_DOUBLE;
	}

	inter.computeIntersect = false;
	
	int stack[64];
	//double minT = tools::MAX_DOUBLE;
	int currentNode = 0;
	
	int dirIsNeg[3] = { _ray.invDir().x() < 0,  _ray.invDir().y() < 0,  _ray.invDir().z() < 0 };

	int stackIndex = 0;

	//Adaptative ray epsilon
	Ray ray(_ray);
	if (ray.myMinT == tools::EPSILON)
		ray.myMinT = std::max(ray.myMinT, ray.myMinT * ray.myOrigin.array().abs().maxCoeff());

	if (myNodes.empty() || ray.myMaxT < ray.myMinT)
		return false;

	bool hasHit = false;

	while (true)
	{
		const LinearBVHNode& node = myNodes[currentNode];

		double nearT, farT;
		if (!node.myBoundingBox.intersection(ray, nearT, farT))
		{
			if (stackIndex == 0)
				break;
			currentNode = stack[--stackIndex];
			continue;
		}

		//Inner node
		if (node.myRightChild != -1)
		{
			//Choose the node less likely to be intersected
			if (dirIsNeg[node.myAxis])
			{
				stack[stackIndex++] = currentNode + 1;
				currentNode = node.myRightChild;
			}
			else
			{
				stack[stackIndex++] = node.myRightChild;
				currentNode++;
				assert(stackIndex < 64);
			}
		}
		else
		{
			for (auto i = node.myFirstPrimIndex; i < node.myFirstPrimIndex + node.myNumberPrimitives; i++)
			{
				if (myPrimitives[i]->intersection(ray, inter, shadowRay))
				{
					if (shadowRay)
						return true;
					ray.myMaxT = inter.t;
					hasHit = true;
				}
			}

			if (stackIndex == 0)
				break;
			currentNode = stack[--stackIndex];
			continue;
		}
	}

	if (hasHit && shouldCompute)
	{
		DifferentialGeometry trueGeometry;
		DifferentialGeometry shadingGeometry;
		inter.myPrimitive->getDifferentialGeometry(trueGeometry, shadingGeometry, inter);
		//inter.myPrimitive->intersection(ray, inter, false);
		inter.myTrueGeometry = trueGeometry;
		inter.myShadingGeometry = shadingGeometry;
	}

	return hasHit;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
//bool Bvh::intersection(const Ray & _ray, Intersection& inter, bool shadowRay)
//{
//	std::vector<StackIntersection> stack;
//	double minT = tools::MAX_DOUBLE;
//	BvhNode::ptr currentNode = root;
//
//	double tmp1, tmp2;
//
//	//Adaptative ray epsilon
//	Ray ray(_ray);
//	if (ray.myMinT == tools::EPSILON)
//		ray.myMinT = std::max(ray.myMinT, ray.myMinT * ray.myOrigin.array().abs().maxCoeff());
//
//	bool hasHit = false;
//
//	if (!currentNode->myBoundingBox.intersection(ray, tmp1, tmp2))
//	{
//		return false;
//	}
//
//	while (true)
//	{
//		if (currentNode->myChildren[0] != nullptr && currentNode->myChildren[1] != nullptr)
//		{
//			bool interChild1 = false;
//			bool interChild2 = false;
//			double tNearC1, tNearC2;
//			if (currentNode->myChildren[0]->myBoundingBox.intersection(ray, tNearC1, tmp2))
//			{
//				interChild1 = true;
//			}
//			if (currentNode->myChildren[1]->myBoundingBox.intersection(ray, tNearC2, tmp2))
//			{
//				interChild2 = true;
//			}
//
//			if (interChild1 && interChild2)
//			{
//				if (tNearC1 < tNearC2)
//				{
//					stack.push_back(StackIntersection(currentNode->myChildren[1], tNearC2));
//					currentNode = currentNode->myChildren[0];
//					continue;
//				}
//				else
//				{
//					stack.push_back(StackIntersection(currentNode->myChildren[0], tNearC1));
//					currentNode = currentNode->myChildren[1];
//					continue;
//				}
//			}
//			else if (!interChild1 && !interChild2)
//			{
//
//			}
//			else
//			{
//				if (interChild1)
//				{
//					currentNode = currentNode->myChildren[0];
//				}
//				else
//				{
//					currentNode = currentNode->myChildren[1];
//				}
//				continue;
//			}
//		}
//		else
//		{
//			for (auto i = currentNode->myFirstPrimIndex; i < currentNode->myFirstPrimIndex + currentNode->myNumberPrimitives; i++)
//			{
//				if (myPrimitives[i]->intersection(ray, inter, shadowRay))
//				{
//					minT = ray.myMaxT = inter.t;
//					hasHit = true;
//				}
//			}
//		}
//
//		bool shouldPop = true;
//		while (shouldPop)
//		{
//			if (stack.empty())
//			{
//				return hasHit;
//			}
//			StackIntersection interStack = stack.back();
//			stack.pop_back();
//			if (interStack.t < minT)
//			{
//				currentNode = interStack.node;
//				shouldPop = false;
//			}	
//		}
//	}
//
//	return hasHit;
//}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
int Bvh::flatten(BvhNode & node, int* current)
{
	LinearBVHNode& linearNode = myNodes[*current];

	int offset = (*current)++;
	linearNode.myBoundingBox = node.myBoundingBox;
	
	if (node.myChildren[0] == nullptr && node.myChildren[1] == nullptr)
	{
		linearNode.myFirstPrimIndex = node.myFirstPrimIndex;
		linearNode.myNumberPrimitives = node.myNumberPrimitives;
	}
	else
	{
		linearNode.myAxis = node.myAxis;
		flatten(*node.myChildren[0], current);//current + 1);
		linearNode.myRightChild = flatten(*node.myChildren[1], current);//current + 1);
	}

	return offset;//current;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void Bvh::buildBinNode(int startIndex, int endIndex, BvhNode::ptr currentNode, const std::vector<BvhData, Eigen::aligned_allocator<BvhData>>& data, int depth)
{
	myNumberOfNodes++;

	int number = endIndex - startIndex;

	if (number < 32) 
	{
		buildNode(startIndex, endIndex, currentNode, data, depth);
		return;
	}

	Axis axis = getLargestAxisExtent(currentNode->myBoundingBox);

	double min = currentNode->myBoundingBox.getMin()[axis];
	double max = currentNode->myBoundingBox.getMax()[axis];
	double binConstant = Bin::BIN_NUMBER / (max - min);

	currentNode->myAxis = axis;

	Bin bins[Bin::BIN_NUMBER];
	
	const int nbThreads = 7;
	Bin tmpBin[nbThreads * Bin::BIN_NUMBER];

	//auto binCreation = [&](int minT, int maxT, int threadNumber)
	{
		//Create the bins
		//for (auto i = minT; i < maxT; i++)
		//{
		//	BoundingBox box = myPrimitives[i]->getWorldBoundingBox();
		//	int index = std::min(std::max((int)((myPrimitives[i]->getCentroid()[axis] - min) * binConstant), 0), Bin::BIN_NUMBER - 1);
		//	tmpBin[index + Bin::BIN_NUMBER * threadNumber].myBox = BoundingBox::unionBox(tmpBin[index + Bin::BIN_NUMBER * threadNumber].myBox, box);
		//	tmpBin[index + Bin::BIN_NUMBER * threadNumber].myNumberPrimitives++;
		//}

		////Create the bins
		for (auto i = startIndex; i < endIndex; i++)
		{
			BoundingBox box = myPrimitives[i]->getWorldBoundingBox();
			int index = std::min(std::max((int)((myPrimitives[i]->getCentroid()[axis] - min) * binConstant), 0), Bin::BIN_NUMBER - 1);
			bins[index].myBox = BoundingBox::unionBox(bins[index].myBox, box);
			bins[index].myNumberPrimitives++;
		}
	};
	//std::unique_ptr<std::thread> threads[nbThreads];
	//int tmpStart = startIndex;
	//for (auto i = 0; i < nbThreads; i++)
	//{
	//	int threadSize = number / nbThreads;
	//	if(i == nbThreads - 1)
	//		threads[i] = std::unique_ptr<std::thread>(new std::thread(binCreation, tmpStart, endIndex, i));
	//	else
	//		threads[i] = std::unique_ptr<std::thread>(new std::thread(binCreation, tmpStart, tmpStart + threadSize, i));
	//	tmpStart = tmpStart + threadSize;
	//}
	//for (auto i = 0; i < nbThreads; i++)
	//{
	//	threads[i]->join();
	//}

	//for (auto i = 0; i < nbThreads; i++)
	//{
	//	for (auto j = 0; j < Bin::BIN_NUMBER; j++)
	//	{
	//		bins[j].myNumberPrimitives += tmpBin[j + Bin::BIN_NUMBER * i].myNumberPrimitives;
	//		bins[j].myBox = BoundingBox::unionBox(bins[j].myBox, tmpBin[j + Bin::BIN_NUMBER * i].myBox);
	//	}
	//}

	double minCost = INTERSECTION_COST * number;
	int bestIndex = -1;

	BoundingBox leftBox[Bin::BIN_NUMBER];
	leftBox[0] = bins[0].myBox;
	for (auto i = 1; i < Bin::BIN_NUMBER; i++)
	{
		bins[i].myNumberPrimitives += bins[i - 1].myNumberPrimitives;
		leftBox[i] = BoundingBox::unionBox(leftBox[i - 1], bins[i].myBox);
	}

	double factor = INTERSECTION_COST / currentNode->myBoundingBox.getSurfaceValue();
	BoundingBox rightBox = bins[Bin::BIN_NUMBER - 1].myBox;
	BoundingBox bestRightBBox;
	for (auto i = Bin::BIN_NUMBER - 2; i >= 0; i--)
	{
		//int index = startIndex + j;
		double leftNumber = bins[i].myNumberPrimitives;
		double rightNumber = number - bins[i].myNumberPrimitives;
		double cost = 2 * TRAVERSAL_COST + factor * (leftNumber * leftBox[i].getSurfaceValue() + rightNumber * rightBox.getSurfaceValue());

		if (cost < minCost)
		{
			minCost = cost;
			bestIndex = i;
			bestRightBBox = rightBox;
		}
		rightBox = BoundingBox::unionBox(rightBox, bins[i].myBox);
	}

	if (bestIndex == -1)
	{
		buildNode(startIndex, endIndex, currentNode, data, depth);
		return;
	}
	else
	{
		uint32_t leftCount = bins[bestIndex].myNumberPrimitives;
		
		//std::partition(myPrimitives.begin() + startIndex, myPrimitives.begin() + endIndex, CmpBin(binConstant, min, bestIndex, axis));
		
		std::vector<IPrimitive::ptr> tmp(number);
		//std::atomic_int tmpLeft = 0;
		//std::atomic_int tmpRight = leftCount;
		int tmpLeft = 0;
		int tmpRight = leftCount;
		//concurrency::parallel_for(startIndex, endIndex, [&] (size_t i) 
		for (auto i = startIndex; i < endIndex; i++)
		{
			BoundingBox box = myPrimitives[i]->getWorldBoundingBox();
			int index = std::min(std::max((int)((myPrimitives[i]->getCentroid()[axis] - min) * binConstant), 0), Bin::BIN_NUMBER - 1);
			
			if (index <= bestIndex)
			{
				tmp[tmpLeft++] = myPrimitives[i];
				//tmpLeft++;
			}
			else
			{
				tmp[tmpRight++] = myPrimitives[i];
				//tmpRight++;
			}
		}//);

		std::copy(tmp.begin(), tmp.end(), myPrimitives.begin() + startIndex);
			
		BvhNode::ptr left = BvhNode::ptr(new BvhNode);
		left->myBoundingBox = leftBox[bestIndex];
		BvhNode::ptr right = BvhNode::ptr(new BvhNode);
		right->myBoundingBox = bestRightBBox;

		currentNode->addChildren(left, right);

		//std::cout << startIndex << " " << bestIndex << " "<<  leftCount << " " << endIndex << " " << numberNode << std::endl;

		buildBinNode(startIndex, startIndex + leftCount, left, data, depth + 1);
		buildBinNode(startIndex + leftCount, endIndex, right, data, depth + 1);
	}
}
