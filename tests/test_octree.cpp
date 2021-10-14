#include "testutils.hpp"
#include <engine/utils/containers/octree.hpp>
#include <glm/glm.hpp>

using namespace glm;

template<typename TreeT>
struct Processor
{
	using AABB = typename TreeT::AABB;
	bool descend(const AABB& box)
	{
		++descends;
		return true;
	}

	void process(const AABB& box, int val)
	{
		found.emplace_back(box, val);
		++processed;
	}

	void reset()
	{
		found.clear();
		descends = 0;
		processed = 0;
	}

	std::vector<std::pair<AABB, int>> found;
	int descends = 0;
	int processed = 0;
};

int testOctree2D()
{
	using TreeT = utils::SparseOctree<int, 2, float>;

	TreeT tree(1.f);
	std::vector<std::pair<TreeT::AABB, int>> expectedElements;
	Processor<TreeT> proc;
	int counter = 0;
	auto insert = [&](const TreeT::AABB& aabb, int i)
	{
		tree.insert(aabb, i);
		expectedElements.emplace_back(aabb, i);
	};

	insert({ vec2(0.25f), vec2(0.75f) }, counter++);
	tree.traverse(proc);
	EXPECT(proc.descends == 1 && proc.processed == 1 && proc.found.size() == expectedElements.size(), "Insert element in root node.");
	for (auto& el : expectedElements)
		EXPECT(std::find(proc.found.begin(), proc.found.end(), el) != proc.found.end(), "Inserted elements can be retrieved.");

	proc.reset();
	insert({ vec2(0.0f), vec2(0.5f, 0.51f) }, counter++);
	tree.traverse(proc);
	EXPECT(proc.descends == 1 && proc.processed == 2, "Insert element at upper edge.");
	for (auto& el : expectedElements)
		EXPECT(std::find(proc.found.begin(), proc.found.end(), el) != proc.found.end(), "Inserted elements can be retrieved.");

	proc.reset();
	insert({ vec2(0.0f), vec2(0.5f) }, counter++);
	tree.traverse(proc);
	EXPECT(proc.descends == 2 && proc.processed == 3, "Insert subdividing element.");
	for (auto& el : expectedElements)
		EXPECT(std::find(proc.found.begin(), proc.found.end(), el) != proc.found.end(), "Inserted elements can be retrieved.");

	for (int i = 0; i < 16; ++i)
		tree.insert({ vec2(static_cast<float>(i) + 0.1f), vec2(static_cast<float>(i) + 1.51f) }, counter++);
	TreeT::AABBQuery query({ vec2(0.f, 4.f), vec2(42000.f, 5.f) });
	tree.traverse(query);
	EXPECT(query.hits.size() == 2, "AABB query.");
	EXPECT(std::find(query.hits.begin(), query.hits.end(), 6) != query.hits.end(), "AABB query.");
	EXPECT(std::find(query.hits.begin(), query.hits.end(), 7) != query.hits.end(), "AABB query.");

	proc.reset();
	EXPECT(tree.remove({ vec2(0.0f), vec2(0.49f) }, 2), "Remove existing element.");
	EXPECT(!tree.remove({ vec2(0.0f), vec2(0.49f) }, 2), "Remove not existing element.");
	tree.traverse(proc);
	EXPECT(proc.descends == 18 && proc.processed == 18, "Remove element.");
	for (auto& el : expectedElements)
	{
		if(el.second == 2)
			EXPECT(std::find(proc.found.begin(), proc.found.end(), el) == proc.found.end(), "Removed element is gone.");
		else
			EXPECT(std::find(proc.found.begin(), proc.found.end(), el) != proc.found.end(), "Inserted elements can be retrieved.");
	}


	return testsFailed;
}

void testOctree3D()
{
	using TreeT = utils::SparseOctree<int, 2, double>;

	int testsFailed = 0;

	TreeT tree(1.f);
	std::vector<std::pair<TreeT::AABB, int>> expectedElements;
	Processor<TreeT> proc;

	auto insert = [&](const TreeT::AABB& aabb, int i)
	{
		tree.insert(aabb, i);
		expectedElements.emplace_back(aabb, i);
	};

	insert({ dvec3(0.21), dvec3(0.5, 0.25, 0.8) }, 2);
	insert({ dvec3(1.0), dvec3(3.0) }, 3);
	insert({ dvec3(0.1, 0.5, 0.8), dvec3(0.2, 0.7, 0.9) }, 3);
	insert({ dvec3(0.2), dvec3(0.3) }, 1);
	tree.traverse(proc);
	for (auto& el : expectedElements)
		EXPECT(std::find(proc.found.begin(), proc.found.end(), el) != proc.found.end(), "Inserted elements can be retrieved in 3D.");

	tree.remove({ dvec3(0.2), dvec3(0.3) }, 1);
	proc.reset();
	tree.traverse(proc);
	expectedElements.pop_back();
	for (auto& el : expectedElements)
		EXPECT(std::find(proc.found.begin(), proc.found.end(), el) != proc.found.end(), "3D tree is consistent after removal.");

}

int main() 
{
	testOctree2D();
	testOctree3D();

	return testsFailed;
}
