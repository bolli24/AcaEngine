#include "testutils.hpp"

#pragma error include registry file!!
#include <optional>
#include <vector>

struct Foo {
	int i;
};

struct Bar {
	float f;
};

int main()
{

	Registry registry;

	std::vector<Entity> entities;

	for (int i = 0; i < 5; ++i)
		entities.push_back(registry.create());

	auto refDel = registry.getRef(entities[2]);

	EXPECT(registry.getEntity(refDel), "Reference is valid after creation.");
	registry.erase(entities[2]);
	EXPECT(!registry.getEntity(refDel), "Reference is invalid after delete.");

	entities[2] = registry.create();
	for (int i = 0; i < 6; ++i)
		entities.push_back(registry.create());

	EXPECT(!registry.getEntity(refDel), "Reference remains invalid after reuse of the id.");

	{
		CompomentStorageAccess<Foo>& fooComps = registry.getComponents<Foo>();
		for (int i = 0; i < static_cast<int>(entities.size()); ++i)
		{
			const Foo& foo = fooComps.insert(entities[i], Foo{i});
			EXPECT(foo.i == i, "Add a component.");
		}
	}

	{
		CompomentStorageAccess<Bar>& barComps = registry.getComponents<Bar>();
		for (int i = 0; i < static_cast<int>(entities.size()); i += 3){
			const Bar& bar = barComps.insert(entities[i], Bar{static_cast<float>(i)});
			EXPECT(bar.f == static_cast<float>(i), "Add a component.");
		}
	}

	{
		CompomentStorageAccess<Foo>& fooComps = registry.getComponents<Foo>();
		const CompomentStorageAccess<Foo>& constFooComps = fooComps;
		for (int i = 0; i < static_cast<int>(entities.size()); ++i)
		{
			EXPECT(fooComps.at(entities[i]), "Retrieve a component.");
			EXPECT(constFooComps.at(entities[i]), "Retrieve a component.");
		}

		fooComps.erase(entities[0]);
		fooComps.erase(entities[1]);

		EXPECT(!fooComps.at(entities[0]), "Remove a component.");
		EXPECT(!fooComps.at(entities[1]), "Remove a component.");


		registry.erase(entities[2]);

		for (int i = 3; i < static_cast<int>(entities.size()); ++i)
		{
			EXPECT(fooComps.at(entities[i]), "Other components are untouched.");
			EXPECT(fooComps.at(entities[i])->i == i, "Other components are untouched.");
		}
	}

	int sum = 0;
	registry.execute([&sum](const Foo& foo) { sum += foo.i; });
	// without auto deduction
	// registry.execute<Foo>([&sum](const Foo& foo) { sum += foo.i; });
	EXPECT(sum == 10 * 11 / 2 - 3, "Execute action on a single component type.");

	sum = 0;
	registry.execute([&sum](const Bar& bar, const Foo& foo) { sum += foo.i - 2 * static_cast<int>(bar.f); });
	//registry.execute<Bar,Foo>([&sum](const Bar& bar, const Foo& foo) { sum += foo.i - 2 * static_cast<int>(bar.f); });
	EXPECT(sum == -3 - 6 - 9, "Execute action on multiple component types.");

	// registry.execute<Entity, Bar>([&](Entity ent, Bar& bar)
	registry.execute([&](Entity ent, Bar& bar)
		{
			auto pBar = registry.getComponents<Bar>().at(ent);
			EXPECT(pBar, "Execute provides the correct entity.");
			EXPECT(pBar->f == bar.f, "Execute provides the correct entity.");
			bar.f = -1.f;
		});

	{
		const CompomentStorageAccess<Bar>& barComps = registry.getComponents<Bar>();
		for (size_t i = 3; i < entities.size(); i+=3)
		{
			auto pBar = barComps.at(entities[i]);
			EXPECT(pBar, "Action can change components.");
			EXPECT(pBar->f == -1.f, "Action can change components.");
		}
	}
}
