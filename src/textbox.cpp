#include "textbox.hpp"
#include "util/stringutils.hpp"

namespace sum
{

TextBox::TextBox(
	sf::RenderWindow *_window,
	int _x,
	int _y,
	int _width,
	int _height,
	int _size) :
Widget(_window, _x, _y,_width,_height),
size(_size),
linesize(13),
chopping(false),
chopping_size(0)
{
	text.SetX(x);
	text.SetY(y);
	text.SetSize(textSize);
	tmp.SetSize(textSize);
}

void TextBox::draw()
{
	text.SetColor(textColor);
	size_t i(0),j(0);
	int chopped(0);
	for(
		chopping == true ? chopped=i=lines.size()-chopping_size : i=0;
		i<lines.size();
		++i
	)
	{
		chopping == true ? j=i-chopped : j=i;
		text.SetText(lines[i]);
		text.SetY(y + j*linesize);
		window->Draw(text);
	}
}

void TextBox::add(std::string _text)
{
	_text = stringutils::tabconv(_text);
	tmp.SetText(_text);
	if(tmp.GetRect().GetWidth() > width-linesize)
	{
		std::string buff="";
		int breaks(0);
		for(size_t k=0; k<_text.size() ; ++k)
		{
			buff+=_text[k];
			tmp.SetText(buff);
			if(tmp.GetRect().GetWidth() > width-linesize)
			{
				lines.push_back(buff);
				buff="";
				++breaks;
			}

		}
		if(!buff.empty())
		{
			lines.push_back(buff);
		}
	}
	else
	{
		lines.push_back(_text);
	}
	if(chopping ==false && lines.size()*linesize > static_cast<size_t>(height))
	{
		chopping=true;
		chopping_size=lines.size();
	}
}

std::vector<std::string> TextBox::val()
{
	return lines;
}

}
