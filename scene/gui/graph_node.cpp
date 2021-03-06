#include "graph_node.h"


bool GraphNode::_set(const StringName& p_name, const Variant& p_value) {

	if (!p_name.operator String().begins_with("slot/"))
		return false;

	int idx=p_name.operator String().get_slice("/",1).to_int();
	String what = p_name.operator String().get_slice("/",2);


	Slot si;
	if (slot_info.has(idx))
		si=slot_info[idx];


	if (what=="left_enabled")
		si.enable_left=p_value;
	else if (what=="left_type")
		si.type_left=p_value;
	else if (what=="left_color")
		si.color_left=p_value;
	else if (what=="right_enabled")
		si.enable_right=p_value;
	else if (what=="right_type")
		si.type_right=p_value;
	else if (what=="right_color")
		si.color_right=p_value;
	else
		return false;

	set_slot(idx,si.enable_left,si.type_left,si.color_left,si.enable_right,si.type_right,si.color_right);
	update();
	return true;
}

bool GraphNode::_get(const StringName& p_name,Variant &r_ret) const{


	print_line("get "+p_name.operator String());
	if (!p_name.operator String().begins_with("slot/")) {
		print_line("no begins");
		return false;
	}

	int idx=p_name.operator String().get_slice("/",1).to_int();
	String what = p_name.operator String().get_slice("/",2);



	Slot si;
	if (slot_info.has(idx))
		si=slot_info[idx];

	if (what=="left_enabled")
		r_ret=si.enable_left;
	else if (what=="left_type")
		r_ret=si.type_left;
	else if (what=="left_color")
		r_ret=si.color_left;
	else if (what=="right_enabled")
		r_ret=si.enable_right;
	else if (what=="right_type")
		r_ret=si.type_right;
	else if (what=="right_color")
		r_ret=si.color_right;
	else
		return false;

	print_line("ask for: "+p_name.operator String()+" get: "+String(r_ret));
	return true;
}
void GraphNode::_get_property_list( List<PropertyInfo> *p_list) const{

	int idx=0;
	for(int i=0;i<get_child_count();i++) {
		Control *c=get_child(i)->cast_to<Control>();
		if (!c || c->is_set_as_toplevel() || !c->get_owner())
			continue;

		String base="slot/"+itos(idx)+"/";

		p_list->push_back(PropertyInfo(Variant::BOOL,base+"left_enabled"));
		p_list->push_back(PropertyInfo(Variant::INT,base+"left_type"));
		p_list->push_back(PropertyInfo(Variant::COLOR,base+"left_color"));
		p_list->push_back(PropertyInfo(Variant::BOOL,base+"right_enabled"));
		p_list->push_back(PropertyInfo(Variant::INT,base+"right_type"));
		p_list->push_back(PropertyInfo(Variant::COLOR,base+"right_color"));

		idx++;
	}
}


void GraphNode::_resort() {



	int sep=get_constant("separation");
	Ref<StyleBox> sb=get_stylebox("frame");
	bool first=true;

	Size2 minsize;

	for(int i=0;i<get_child_count();i++) {
		Control *c=get_child(i)->cast_to<Control>();
		if (!c)
			continue;
		if (c->is_set_as_toplevel())
			continue;

		Size2i size=c->get_combined_minimum_size();

		minsize.y+=size.y;
		minsize.x=MAX(minsize.x,size.x);

		if (first)
			first=false;
		else
			minsize.y+=sep;

	}

	int vofs=0;
	int w = get_size().x - sb->get_minimum_size().x;


	cache_y.clear();
	for(int i=0;i<get_child_count();i++) {
		Control *c=get_child(i)->cast_to<Control>();
		if (!c)
			continue;
		if (c->is_set_as_toplevel() || !c->get_owner())
			continue;

		Size2i size=c->get_combined_minimum_size();

		Rect2 r(sb->get_margin(MARGIN_LEFT),sb->get_margin(MARGIN_TOP)+vofs,w,size.y);

		fit_child_in_rect(c,r);
		cache_y.push_back(vofs+size.y*0.5);

		if (vofs>0)
			vofs+=sep;
		vofs+=size.y;


	}

	_change_notify();
	update();

}


void GraphNode::_notification(int p_what) {

	if (p_what==NOTIFICATION_DRAW) {

		Ref<StyleBox> sb=get_stylebox("frame");
		Ref<Texture> port =get_icon("port");
		Point2i icofs = -port->get_size()*0.5;
		int edgeofs=3;
		icofs.y+=sb->get_margin(MARGIN_TOP);
		draw_style_box(sb,Rect2(Point2(),get_size()));

		for (Map<int,Slot>::Element *E=slot_info.front();E;E=E->next()) {

			if (E->key()>cache_y.size())
				continue;
			if (!slot_info.has(E->key()))
				continue;
			const Slot &s=slot_info[E->key()];
			//left
			if (s.enable_left)
				port->draw(get_canvas_item(),icofs+Point2(edgeofs,cache_y[E->key()]),s.color_left);
			if (s.enable_right)
				port->draw(get_canvas_item(),icofs+Point2(get_size().x-edgeofs,cache_y[E->key()]),s.color_right);

		}
	}
	if (p_what==NOTIFICATION_SORT_CHILDREN) {

		_resort();
	}

}

void GraphNode::set_title(const String& p_title) {

	title=p_title;
	update();
}

String GraphNode::get_title() const {

	return title;
}

void GraphNode::set_slot(int p_idx,bool p_enable_left,int p_type_left,const Color& p_color_left, bool p_enable_right,int p_type_right,const Color& p_color_right) {

	ERR_FAIL_COND(p_idx<0);

	if (!p_enable_left && p_type_left==0 && p_color_left==Color(1,1,1,1) && !p_enable_right && p_type_right==0 && p_color_right==Color(1,1,1,1)) {
		slot_info.erase(p_idx);
		return;
	}

	Slot s;
	s.enable_left=p_enable_left;
	s.type_left=p_type_left;
	s.color_left=p_color_left;
	s.enable_right=p_enable_right;
	s.type_right=p_type_right;
	s.color_right=p_color_right;
	slot_info[p_idx]=s;
	update();
}

void GraphNode::clear_slot(int p_idx){

	slot_info.erase(p_idx);
	update();
}
void GraphNode::clear_all_slots(){

	slot_info.clear();
	update();
}
bool GraphNode::is_slot_enabled_left(int p_idx) const{

	if (!slot_info.has(p_idx))
		return false;
	return slot_info[p_idx].enable_left;

}

int GraphNode::get_slot_type_left(int p_idx) const{

	if (!slot_info.has(p_idx))
		return 0;
	return slot_info[p_idx].type_left;

}

Color GraphNode::get_slot_color_left(int p_idx) const{

	if (!slot_info.has(p_idx))
		return Color(1,1,1,1);
	return slot_info[p_idx].color_left;

}

bool GraphNode::is_slot_enabled_right(int p_idx) const{

	if (!slot_info.has(p_idx))
		return false;
	return slot_info[p_idx].enable_right;

}



int GraphNode::get_slot_type_right(int p_idx) const{

	if (!slot_info.has(p_idx))
		return 0;
	return slot_info[p_idx].type_right;

}

Color GraphNode::get_slot_color_right(int p_idx) const{

	if (!slot_info.has(p_idx))
		return Color(1,1,1,1);
	return slot_info[p_idx].color_right;

}

Size2 GraphNode::get_minimum_size() const {

	int sep=get_constant("separation");
	Ref<StyleBox> sb=get_stylebox("frame");
	bool first=true;

	Size2 minsize;

	for(int i=0;i<get_child_count();i++) {

		Control *c=get_child(i)->cast_to<Control>();
		if (!c)
			continue;
		if (c->is_set_as_toplevel() || !c->get_owner())
			continue;

		Size2i size=c->get_combined_minimum_size();

		minsize.y+=size.y;
		minsize.x=MAX(minsize.x,size.x);

		if (first)
			first=false;
		else
			minsize.y+=sep;
	}

	return minsize+sb->get_minimum_size();
}


void GraphNode::_bind_methods() {


}

GraphNode::GraphNode() {


}
