#include "filterpipeline.h"
#include "filter.h"

namespace udg {

FilterPipeline::FilterPipeline( )
{
	m_numFilters=0;

}

FilterPipeline::~FilterPipeline()
{
}
/* M�tode per afegir un filtre*/
	void FilterPipeline::push(Filter * newFilter)
	{
		m_filterStack.push(newFilter);
		m_numFilters++;

	}

	/* Metode per treure un filtre de la pila */
	Filter * FilterPipeline::pop()
	{
		Filter *filter;
		if (m_numFilters >0)
		{
			
			filter = m_filterStack.pop();
			m_numFilters--;
		}
		else
		{
			filter = NULL;
		}
		return filter;
	}
	
	/* Metode que retorna l'�ltim filtre afegit */
	Filter * FilterPipeline::top()
	{
		Filter *filter;
		if (m_numFilters >0)
		{			
			filter = m_filterStack.top();			
		}
		else
		{
			filter = NULL;
		}
		return filter;
		
	}

	int FilterPipeline::getNumberOfFilters()
	{
		return m_numFilters;
	}
}