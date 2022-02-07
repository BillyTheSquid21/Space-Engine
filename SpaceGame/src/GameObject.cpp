#include "GameObject.h"

void GameObject::setRenderer(Renderer<Vertex>* ren) {
	m_Renderer = ren;
}

void GameObject::render() {
	return;
}

void GameObject::update(double deltaTime) {
	return;
}