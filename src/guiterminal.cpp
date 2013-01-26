
#include "guiterminal.hpp"
#include <iostream>

namespace sum{

GuiTerminal::GuiTerminal(
	sf::RenderWindow *_window,
	std::string _player_name,
	int _width,
	int _height) :
Widget(_window),
player_name(_player_name),
inputfield_size(25),
frozen(false),
completion_init(false)
{
	window=_window;

	if(_width!=0)
		width=_width;
	else
		width=_window->GetWidth()-(_window->GetWidth()/3);
	if(_height!=0)
		height=_height;
	else
		height=_window->GetHeight()/3;

	term=new Terminal();
	x=5;
	y=window->GetHeight()-height;
	name_pwd.SetX(x);
	name_pwd.SetY(y+height-inputfield_size);
	name_pwd.SetSize(textSize);
	name_pwd.SetText(player_name + term->get_working_directory() + "$");
	inputfield=new InputField(
		_window,
		x+name_pwd.GetRect().GetWidth(),
		y+height-inputfield_size,
		width-x-name_pwd.GetRect().GetWidth()-x,
		inputfield_size);
	textbox=new TextBox(_window,x,y,width,height-(inputfield_size*2));

}

GuiTerminal::~GuiTerminal()
{
	delete inputfield;
	delete textbox;
	delete term;
}

void GuiTerminal::update(const ServerMessage &message)
{
	sf::Lock Lock(mutex);
	std::vector<std::string> ret;
	switch(message.type)
	{
		case ServerMessage::server_fun:
			term->add_server_exe("/bin", message.msg, message.msg);
			break;
		case ServerMessage::register_mons:
			ret = message.get_parsed_msg();
			if(ret.size() > 1) term->add_readable("/mon", ret[0], ret[1]);
			break;
		case ServerMessage::reply:
			frozen=false;
			if(!message.msg.empty()) {
				ret = string_explode(message.msg, "\n");
				for(size_t i=0; i<ret.size(); ++i) if(i != ret.size() -1 || !ret[i].empty()) textbox->add(ret[i]);
			}
			break;
		default:
			break;
	}
}

void GuiTerminal::draw()
{
	inputfield->draw();
	name_pwd.SetColor(nameColor);
	window->Draw(name_pwd);
	textbox->draw();
}

void GuiTerminal::handleEvent(sf::Event &event)
{
	sf::Lock Lock(mutex);
	if(!frozen)
	{
		if(event.Type == sf::Event::KeyPressed && event.Key.Code != sf::Key::Tab) completion_init = false;

		if((event.Key.Code == sf::Key::Return) && (event.Type == sf::Event::KeyPressed))
		{
			buffer.enter(inputfield->val());
			std::string term_user=player_name + term->get_working_directory() + "$";
			textbox->add(term_user+inputfield->val());
			std::vector<std::string> ret = string_explode(term->command(inputfield->val()), '\n');

			if(ret.size() > 0 && ret[0] == Terminal::freezing_return) frozen=true;

			term_user=player_name + term->get_working_directory() + "$";
			name_pwd.SetText(term_user);
			inputfield->setX(x + name_pwd.GetRect().GetWidth());
			inputfield->setWidth(width-x-name_pwd.GetRect().GetWidth()-x);

			if(frozen)
				textbox->add("Waiting for server to reply...");
			else
			{
				for(size_t i=0; i<ret.size(); ++i)
					if(i != ret.size() -1 || !ret[i].empty()) textbox->add(ret[i]);
			}

			inputfield->reset();
		}
		if(event.Key.Code==sf::Key::Tab && event.Type == sf::Event::KeyPressed) {
			std::string frag1, frag2;
			frag1 = inputfield->val().substr(0, inputfield->getPos());
			if( static_cast<size_t>(inputfield->getPos()) <= inputfield->val().size()) frag2 = inputfield->val().substr(inputfield->getPos());

			//looking only at frag1, we need to separate the part we actually want to complete "(command? <frag>*)frag."
			std::string completable, prefix;
			size_t space = frag1.find_last_of(stringutils::whitespace);
			if(space != std::string::npos) {
				prefix = frag1.substr(0,space+1);
				completable = frag1.substr(space+1); //minden az utolsó space után.
			} else completable = frag1;

			std::set<std::string> res=term->complete(prefix, completable);

			if(res.size() == 1) {
				std::string result = prefix + *(res.begin());
				inputfield->set( result );
				inputfield->setPos( result.size() );
			}
			else if(!completion_init && res.size()>1) {
				completion_init = true;
			}
			else if(completion_init && res.size() > 1) {
				for(std::set<std::string>::const_iterator it=res.begin(); it!=res.end(); ++it) textbox->add(*it);
				textbox->add(player_name + term->get_working_directory() + "$" + inputfield->val());
			}
		}
		if((event.Key.Code == sf::Key::Up) && (event.Type == sf::Event::KeyPressed))
		{
			if(buffer.up())
				inputfield->set(buffer.val());
		}
		if((event.Key.Code == sf::Key::Down) && (event.Type == sf::Event::KeyPressed))
		{
			if(buffer.down())
				inputfield->set(buffer.is_end() ? "" : buffer.val());
		}
		inputfield->handleEvent(event);
	}
}

bool GuiTerminal::Buffer::up()
{
	bool not_start=act!=buff.begin();
	if(not_start)
		--act;
	return not_start;
}

std::string GuiTerminal::Buffer::val()
{
	return *act;
}

bool GuiTerminal::Buffer::is_end()
{
	return act==buff.end();
}

bool GuiTerminal::Buffer::down()
{
	bool not_end=act!=buff.end();
	if(not_end)
		++act;
	return not_end;
}

void GuiTerminal::Buffer::enter(std::string _val)
{
	if(buff.size()>static_cast<size_t>(size))
		buff.erase(buff.begin(), buff.begin() + (size/2));
	buff.push_back(_val);
	act=buff.end();
}


}
