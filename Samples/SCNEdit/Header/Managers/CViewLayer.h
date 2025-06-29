#pragma once

#include "CView.h"

class CViewLayer
{
protected:
	std::vector<CView*> m_views;
	std::vector<CView*> m_willDeleteView;

	CView* m_view;

protected:
	template<class T, typename... Args>
	T* createNewView(Args... args);

public:
	CViewLayer();

	virtual ~CViewLayer();

	void update();

	void render();

	void postRender();

	void onResume();

	void onPause();

	bool onBack();
	void pushView(CView* view);

	bool changeView(CView* view);
	template<class T, typename... Args>
	T* pushView(Args... args);

	template<class T, typename... Args>
	T* changeView(Args... args);

	template<class T, typename... Args>
	T* replaceView(CView* oldView, Args... args);

	void popView();

	void popAllView();

	void popAllViewBefore(CView* s);

	void popAllViewTo(CView* s);

	void removeView(CView* s);

	inline int getViewCount()
	{
		return (int)m_views.size();
	}

	inline CView* getView(int id)
	{
		return m_views[id];
	}

	CView* getCurrentView();

	CView* getViewBefore(CView* state);

	void destroyAllView();
};

template<class T, typename... Args>
T* CViewLayer::createNewView(Args... args)
{
	T* newView = new T(args...);

	CView* view = dynamic_cast<CView*>(newView);
	if (view == NULL)
	{
		char exceptionInfo[512];
		sprintf(exceptionInfo, "CViewLayer::pushView {} must inherit CView", typeid(T).name());
		os::Printer::log(exceptionInfo);

		delete newView;
		return NULL;
	}

	return newView;
}

template<class T, typename... Args>
T* CViewLayer::pushView(Args... args)
{
	T* newView = createNewView<T>(args...);
	if (newView == NULL)
		return NULL;

	if (m_views.size() > 0)
		m_views[0]->onDeactive();

	m_views.insert(m_views.begin(), newView);
	newView->onInit();
	newView->onData();
	return newView;
}

template<class T, typename... Args>
T* CViewLayer::changeView(Args... args)
{
	if (m_views.size() > 0)
	{
		m_willDeleteView.push_back(m_views[0]);
		m_views.erase(m_views.begin());

		T* newView = createNewView<T>(args...);
		if (newView == NULL)
			return NULL;

		m_views.insert(m_views.begin(), newView);
		newView->onInit();
		newView->onData();
		return newView;
	}

	return NULL;
}

template<class T, typename... Args>
T* CViewLayer::replaceView(CView* oldView, Args... args)
{
	for (int i = 0, n = (int)m_views.size(); i < n; i++)
	{
		if (m_views[i] == oldView)
		{
			m_willDeleteView.push_back(m_views[i]);

			m_views.erase(m_views.begin() + i);

			T* newView = createNewView<T>(args...);
			if (newView == NULL)
				return NULL;

			m_views.insert(m_views.begin() + i, newView);
			newView->onInit();
			newView->onData();
			return newView;
		}
	}

	return NULL;
}