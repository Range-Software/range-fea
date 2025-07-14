#ifndef MODEL_IO_H
#define MODEL_IO_H

#include <QObject>
#include <QString>

#include "application_settings.h"
#include <rbl_job.h>

#include "model.h"

typedef enum _ModelIOType
{
    MODEL_IO_NONE = 0,
    MODEL_IO_ADD,
    MODEL_IO_SAVE,
    MODEL_IO_OPEN,
    MODEL_IO_UPDATE,
    MODEL_IO_MSH_EXPORT,
    MODEL_IO_MSH_IMPORT,
    MODEL_IO_RAW_EXPORT,
    MODEL_IO_RAW_IMPORT,
    MODEL_IO_STL_ASCII_EXPORT,
    MODEL_IO_STL_BINARY_EXPORT,
    MODEL_IO_STL_IMPORT,
    MODEL_IO_N_TYPES
} ModelIOType;


class ModelIO : public RJob
{
    Q_OBJECT

    protected:

        //! Type of IO operation.
        ModelIOType  type;
        //! File name.
        QString fileName;
        //! Model ID in case of saving or exporting.
        Model *pModel;

    public:

        explicit ModelIO(ModelIOType    type,
                         const QString &filename,
                         Model         *pModel = nullptr);

        //! Find new model name.
        static QString findNewModelName(const QString &fileName);

        //! Retrieve (opens dialog if necessary) model save filename.
        static QString getModelSaveName(const ApplicationSettings *mainSettings, QWidget *parent, const Model &rModel, bool saveAs = false);

        //! Save model.
        static void save(Model &rModel, const QString &fileName, bool writeLinkFile = true);

        //! Open model.
        static void open(Model &rModel, const QString &fileName);

        //! Update model from file.
        static void update(Model &rModel, const QString &fileName);

        //! Export model to MSH.
        static void exportMsh(Model &rModel, const QString &fileName);

        //! Import model from MSH.
        static void importMsh(Model &rModel, const QString &fileName);

        //! Export model to RAW.
        static void exportRaw(Model &rModel, const QString &fileName);

        //! Import model from RAW.
        static void importRaw(Model &rModel, const QString &fileName);

        //! Export model to STL.
        static void exportStl(Model &rModel, const QString &fileName, bool binary = true);

        //! Import model from STL.
        static void importStl(Model &rModel, const QString &fileName);

    protected:

        virtual int perform() override final;

};

#endif /* MODEL_IO_H */
