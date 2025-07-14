#ifndef TIME_SOLVER_SETUP_WIDGET_H
#define TIME_SOLVER_SETUP_WIDGET_H

#include <QWidget>
#include <QLabel>

#include <rml_time_solver.h>

class TimeSolverSetupWidget : public QWidget
{

    Q_OBJECT

    protected:

        //! Time solver.
        RTimeSolver timeSolver;
        //! End time - readonly.
        QLabel *valueEndTime;

    public:

        //! Constructor.
        explicit TimeSolverSetupWidget(const RTimeSolver &timeSolver, QWidget *parent = nullptr);

    protected:

        //! Find end time.
        double findEndTime() const;

    signals:

        //! Time solver has changed.
        void changed(const RTimeSolver &timeSolver);

    private slots:

        void onTimeSolverEnabledChanged(bool checked);

        void onTimeApproximationChanged(int index);

        void onStartTimeChanged(double startTime);

        void onTimeStepSizeChanged(double timeStepSize);

        void onNTimeStepsChanged(int nTimeSteps);

        void onOutputFrequencyChanged(int outputFrequency);

};

#endif /* TIME_SOLVER_SETUP_WIDGET_H */
