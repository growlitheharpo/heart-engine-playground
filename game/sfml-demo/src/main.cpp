#include <SFML/Graphics.hpp>

#include <heart/stl/vector.h>

int WinMain()
{
	sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);

	hrt::vector<sf::Color> colors = {
		sf::Color::Green,
		sf::Color::Blue,
		sf::Color::Red,
		sf::Color::Yellow,
	};

	hrt::vector<sf::CircleShape> test;
	for (auto c : colors)
	{
		auto& circle = test.emplace_back(25.0f);
		circle.setFillColor(c);
	}

	for (size_t i = 0; i < test.size(); ++i)
	{
		switch (i)
		{
		case 0:
			test[i].setPosition(0.0f, 0.0f);
			break;
		case 1:
			test[i].setPosition(0.0f, 50.0f);
			break;
		case 2:
			test[i].setPosition(50.0f, 0.0f);
			break;
		case 3:
			test[i].setPosition(50.0f, 50.0f);
			break;
		}
	}

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();

		for (auto& s : test)
			window.draw(s);

		window.display();
	}

	return 0;
}
