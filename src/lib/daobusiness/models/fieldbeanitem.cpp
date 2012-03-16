#include "fieldbeanitem.h"
#include "dao/beans/dbfield.h"
#include "dao/beans/dbfieldmetadata.h"
#include <QDate>

FieldBeanItem::FieldBeanItem(DBField *fld) :
		QStandardItem(), m_field(fld)
{

}

// Funciones que deben reimplementarse
int FieldBeanItem::type() const
{
	return QStandardItem::UserType;
}

QVariant FieldBeanItem::data ( int role ) const
{
	Q_UNUSED(role)
	QVariant v = qVariantFromValue((void *) m_field);
	return v;
}

void FieldBeanItem::setData ( const QVariant & v, int role )
{
	Q_UNUSED(role)
	m_field = (DBField *) v.value<void *>();
}

DBField *FieldBeanItem::field()
{
	return m_field;
}

void FieldBeanItem::setField(DBField *field)
{
	m_field = field;
}


bool FieldBeanItem::operator< ( const FieldBeanItem & other ) const
{
	DBField *fld = other.m_field;
	QVariant v = fld->value();
	switch ( fld->metadata()->type() ) {
	case QVariant::Double:
		return (v.toDouble() < m_field->value().toDouble());
	case QVariant::Int:
		return (v.toInt() < m_field->value().toInt());
	case QVariant::Date:
		return (v.toDate() < m_field->value().toDate());
	case QVariant::String:
		return (v.toString() < m_field->value().toString());
	default:
		return (v.toString() < m_field->value().toString());
	}

}
