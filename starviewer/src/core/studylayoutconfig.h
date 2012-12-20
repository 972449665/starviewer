#ifndef UDGSTUDYLAYOUTCONFIG_H
#define UDGSTUDYLAYOUTCONFIG_H

#include <QSet>

class QString;

namespace udg {

class StudyLayoutConfig {
public:
    enum UnfoldType { UnfoldImages, UnfoldSeries };
    enum UnfoldDirectionType { LeftToRightFirst, TopToBottomFirst };
    enum ExclusionCriteriaType { Survey, Localizer };
    
    StudyLayoutConfig();
    /// Constructor amb par�metres m�s habituals
    StudyLayoutConfig(const QString &modality, UnfoldType unfoldType = UnfoldSeries, UnfoldDirectionType unfoldDirection = LeftToRightFirst);
    ~StudyLayoutConfig();

    void setUnfoldType(UnfoldType unfoldType);
    UnfoldType getUnfoldType() const;

    void setUnfoldDirection(UnfoldDirectionType direction);
    UnfoldDirectionType getUnfoldDirection() const;

    void addExclusionCriteria(ExclusionCriteriaType criteria);
    QList<ExclusionCriteriaType> getExclusionCriteria() const;

    void setModality(const QString &modality);
    QString getModality() const;

    void setMaximumNumberOfViewers(int maximum);
    int getMaximumNumberOfViewers() const;

    bool operator ==(const StudyLayoutConfig &config) const;

private:
    /// Nombre m�xim de visors a desplegar. Si �s zero indicaria que s'han de desplegar tants com sigui possible.
    int m_maxNumberOfViewers;

    /// Tipus de desplegament
    UnfoldType m_unfoldType;

    /// Direcci� de desplegament
    UnfoldDirectionType m_unfoldDirection;

    /// Conjunt de criteris d'exclusi�
    QSet<ExclusionCriteriaType> m_exclusionCriteria;

    /// Modalitat a la que aplicar aquesta configuraci�
    QString m_modality;
};

} // End namespace udg

#endif
