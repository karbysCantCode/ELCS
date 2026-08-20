#include "projectmanager.h"
#include "notifications.h"
#include "pingraphicsobject.h"
#include "segmentgraphicsobject.h"
#include "componentgraphicsobject.h"
#include "component.h"
#include "componenttoolbox.h"
#include "filehelper.h"
#include "circuitstyleworkspace.h"
#include "coregates.h"
#include "tutorialmanager.h"
#include "styles.h"

#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

#include <format>


namespace {
    QString stateToDisplayString(States state)
    {
        switch (state) {
            case States::LOW: return "Low";
            case States::HIGH: return "High";
            case States::FLOATING: return "Floating";
            case States::CONFLICT: return "Conflict";
            case States::ERROR: return "Error";
            default: return "Unknown";
        }
    }

    QLabel* makeReadOnlyPropertyLabel(const QString& text)
    {
        auto* label = new QLabel(text);
        label->setStyleSheet(STYLESHEET_LABEL_SECONDARY);
        return label;
    }
}


ProjectManager* globalProjectManager = nullptr;

CircuitWorkspace* __circuitworkspace = nullptr;
CircuitStyleWorkspace* __circuitStyleWorkspace = nullptr;

TutorialOverlay* __tutorialOverlay = nullptr;
TutorialToolbox* __tutorialToobox = nullptr;


ProjectManager::ProjectManager()
    : tutorialManager(
        *__circuitworkspace,
        __tutorialOverlay,
        __tutorialToobox,
        nullptr
    )
{
}


void ProjectManager::onComponentEditRequested(
    const SentinelComponent& component
)
{
    if (currentOpenComponent &&
        !currentOpenComponent->saveToFile(
            currentOpenComponent->getFilePath()
        ))
    {
        globalNotificationManager->notify(
            "Couldn't Save Circuit.",
            std::format(
                "Failed to save circuit \"{}\" before switching to \"{}\".",
                currentOpenComponent->getName(),
                component.getName()
            )
        );
    }

    openComponent(component.getName());
}


void ProjectManager::dummyLoad()
{
    /*
        Built-in core gates.
    */

    auto OR =
        std::make_unique<SentinelComponent>(
            CoreGates::makeOR()
        );

    auto AND =
        std::make_unique<SentinelComponent>(
            CoreGates::makeAND()
        );

    auto XOR =
        std::make_unique<SentinelComponent>(
            CoreGates::makeXOR()
        );

    auto NAND =
        std::make_unique<SentinelComponent>(
            CoreGates::makeNAND()
        );

    auto NOR =
        std::make_unique<SentinelComponent>(
            CoreGates::makeNOR()
        );

    auto XNOR =
        std::make_unique<SentinelComponent>(
            CoreGates::makeXNOR()
        );

    auto NOT =
        std::make_unique<SentinelComponent>(
            CoreGates::makeNOT()
        );

    auto BUFFER =
        std::make_unique<SentinelComponent>(
            CoreGates::makeBUFFER()
        );


    components.emplace(
        OR->getName(),
        std::move(OR)
    );

    components.emplace(
        AND->getName(),
        std::move(AND)
    );

    components.emplace(
        XOR->getName(),
        std::move(XOR)
    );

    components.emplace(
        NAND->getName(),
        std::move(NAND)
    );

    components.emplace(
        NOR->getName(),
        std::move(NOR)
    );

    components.emplace(
        XNOR->getName(),
        std::move(XNOR)
    );

    components.emplace(
        NOT->getName(),
        std::move(NOT)
    );

    components.emplace(
        BUFFER->getName(),
        std::move(BUFFER)
    );


    /*
        Load saved components from resources.
    */

    auto files =
        getFilesInDirectory(
            std::filesystem::path(RESOURCES_PATH)
        );

    for (const auto& file : files)
    {
        if (file.extension() != SAVE_FILE_EXTENSION)
            continue;

        auto [component, success] =
            loadNewComponent(file);

        if (success)
        {
            openComponent(
                component->getName()
            );
        }
    }


    updateToolboxes();


    /*
        Debug/save test.
    */

    auto it =
        components.find("AND");

    if (it != components.end() &&
        !it->second->saveToFile(
            std::filesystem::path(RESOURCES_PATH)
            / "savedand.csf"
        ))
    {
        globalNotificationManager->notify(
            "Couldn't Save Circuit.",
            "Failed to save \"AND\" during dummy load."
        );
    }


    /*
        Start tutorial system.
    */

    tutorialManager.loadTutorial(
        QString(RESOURCES_PATH)
        + QString("/basics.json")
    );

    tutorialManager.start();
}


bool ProjectManager::createNewComponent(
    const std::string& name
)
{
    auto [element, success] =
        components.try_emplace(
            name,
            std::make_unique<SentinelComponent>(name)
        );

    updateToolboxes();

    auto path =
        std::filesystem::path(RESOURCES_PATH)
        / (name + SAVE_FILE_EXTENSION);

    if (!doesFileExist(path))
    {
        createFile(path);

        element->second->setFilePath(path);

        if (!element->second->saveToFile(
                element->second->getFilePath()
            ))
        {
            globalNotificationManager->notify(
                "Couldn't Save Circuit.",
                std::format(
                    "Failed to save newly created circuit \"{}\".",
                    name
                )
            );
        }
    }

    return success;
}


void ProjectManager::updateToolboxes() const
{
    simulatorCircuitToolbox->updateElements();
    styleCircuitToolbox->updateElements();
}


std::pair<SentinelComponent*, bool>
ProjectManager::loadNewComponent(
    const std::filesystem::path& path
)
{
    auto component =
        std::make_unique<SentinelComponent>();

    bool successFileLoad =
        component->loadFromFile(path);

    if (!successFileLoad)
    {
        globalNotificationManager->notify(
            "Failed Loading...",
            std::format(
                "Failed to load component at file \"{}\"",
                path.string()
            ),
            15000
        );

        return { nullptr, false };
    }


    auto [element, success] =
        components.emplace(
            component->getName(),
            std::move(component)
        );

    updateToolboxes();


    /*
        Any sentinel that had an unresolved reference to this
        component can now be resolved.
    */

    for (auto* sentinel : unresolvedSentinelComponents)
    {
        sentinel->informAddedComponentToSeeIfFullyResolved(
            element->second->getName(),
            *element->second
        );
    }


    return {
        element->second.get(),
        success
    };
}


void ProjectManager::registerCallbackOnNewComponent(
    std::function<void()>* func
)
{
    newComponentCallbacks.emplace(func);
}


void ProjectManager::removeCallbackOnNewComponent(
    std::function<void()>* func
)
{
    newComponentCallbacks.erase(func);
}


void ProjectManager::openComponent(
    const std::string& name
)
{
    auto it =
        components.find(name);

    if (it == components.end())
    {
        globalNotificationManager->notify(
            "Error Opening Circuit",
            std::format(
                "Couldn't find circuit \"{}\" in the current project.",
                name
            )
        );

        return;
    }


    currentOpenComponent =
        it->second.get();


    /*
        Remove the currently displayed component.
    */

    removeExistingComponentFromWorkspace();


    /*
        Add the newly opened component.
    */

    addCurrentComponentToWorkspace();
}


void ProjectManager::removeExistingComponentFromWorkspace()
{
    gridManager.reset();
    workspace->reset();
}


void ProjectManager::addCurrentComponentToWorkspace()
{
    std::vector<AbstractPropagator*> rawVec;


    for (const auto& ptr :
         currentOpenComponent->getPropagators())
    {
        rawVec.push_back(
            ptr.get()
        );
    }


    for (auto* propagator : rawVec)
    {
        visuallyRegisterPropagator(
            propagator
        );
    }
}


void ProjectManager::saveCurrentComponent()
{
    if (currentOpenComponent)
    {
        if (!currentOpenComponent->saveToFile(
                currentOpenComponent->getFilePath()
            ))
        {
            globalNotificationManager->notify(
                "Couldn't Save Circuit.",
                std::format(
                    "Failed to save circuit \"{}\".",
                    currentOpenComponent->getName()
                )
            );
        }
    }
    else
    {
        globalNotificationManager->notify(
            "Couldn't Save Circuit.",
            "Couldn't find a currently opened component to save."
        );
    }
}


AbstractPropagator*
ProjectManager::addNewPropagator(
    std::unique_ptr<AbstractPropagator> propagator
)
{
    AbstractPropagator* ptr =
        propagator.get();


    currentOpenComponent->addPropagator(
        std::move(propagator)
    );


    visuallyRegisterPropagator(
        ptr
    );


    return ptr;
}


void ProjectManager::visuallyRegisterPropagator(
    AbstractPropagator* _ptr
)
{
    /*
        COMPONENT
    */

    if (_ptr->isAbstract())
    {
        auto* ptr =
            static_cast<Component*>(_ptr);


        auto pins =
            ptr->getPins();


        for (auto* pin : pins)
        {
            const Position absolutePinPosition =
                ptr->getAbsolutePinPosition(*pin);

            auto touching = globalProjectManager->gridManager.addToGrid(
                absolutePinPosition,
                pin
            );

            pin->propagate();

            for (auto* neighbor : touching) {
                neighbor->propagate();
                neighbor->refreshGraphics();
            }
        }


        /*
            Create graphics object.
        */

        auto* item =
            new ComponentGraphicsObject(
                *ptr
            );


        ptr->setGraphicsObject(
            item
        );


        workspace->scene()->addItem(
            item
        );


        item->setZValue(100);
        item->setVisible(true);
        item->setOpacity(1.0);


        item->setPos(
            ptr->getGridPosition()
                .getGridScaledCopy()
                .getQPointF()
        );


        item->refresh();

        return;
    }


    /*
        NORMAL PROPAGATOR
    */

    auto* ptr =
        static_cast<Propagator*>(_ptr);


    /*
        PIN
    */

    if (ptr->getKind() ==
        Propagator::Kinds::PIN)
    {
        auto* np =
            static_cast<Pin*>(ptr);


        auto touching = globalProjectManager->gridManager.addToGrid(
            np->getGridPosition(),
            np
        );


        auto* item =
            new PinGraphicsObject(
                *np
            );

        np->propagate();
        item->update();

        for (auto* neighbor : touching) {
            neighbor->propagate();
            neighbor->refreshGraphics();
        }


        np->setGraphicsObject(
            item
        );


        workspace->scene()->addItem(
            item
        );


        item->setZValue(1);


        item->setPos(
            np->getGridPosition()
                .getGridScaledCopy()
                .getQPointF()
        );


        return;
    }


    /*
        WIRE
    */

    if (ptr->getKind() ==
        Propagator::Kinds::WIRE)
    {
        auto* np =
            static_cast<Wire*>(ptr);


        auto touchingElements =
            globalProjectManager->gridManager.addToGrid(
                np->segments,
                np
            );


        std::unordered_set<Wire*> tempDeathReg;
        std::unordered_set<Propagator*> excludeSet;


        np->mergeCollidingWires(
            tempDeathReg,
            excludeSet,
            &touchingElements
        );


        np->graphicsObject =
            new SegmentGraphicsObject(
                *np
            );


        np->setGraphicsObject(
            np->graphicsObject
        );


        workspace->scene()->addItem(
            np->graphicsObject
        );


        np->graphicsObject->setZValue(1);

        np->propagate();
        np->graphicsObject->update();

        for (auto* neighbor : touchingElements) {
            if (tempDeathReg.find(static_cast<Wire*>(neighbor)) != tempDeathReg.end())
                continue;

            neighbor->propagate();
            neighbor->refreshGraphics();
        }

        return;
    }
}


void ProjectManager::initiateSimulatorUIPropertyManager(
    PropertySection* _propertySection
)
{
    simulatorPropertySection =
        _propertySection;
}

void ProjectManager::onSelectionCleared()
{
    if (!simulatorPropertySection)
        return;

    simulatorPropertySection->clear();
}

void ProjectManager::onPropagatorSelected(AbstractPropagator* propagator)
{
    if (!simulatorPropertySection || !propagator)
        return;

    simulatorPropertySection->clear();

    if (propagator->isAbstract())
    {
        populateComponentProperties(static_cast<Component*>(propagator));
        return;
    }

    auto* prop = static_cast<Propagator*>(propagator);

    if (prop->getKind() == Propagator::Kinds::PIN)
        populatePinProperties(static_cast<Pin*>(prop));
    else
        populateWireProperties(static_cast<Wire*>(prop));
}

void ProjectManager::populatePinProperties(Pin* pin)
{
    simulatorPropertySection->addProperty(
        "Name",
        makeReadOnlyPropertyLabel(QString::fromStdString(pin->getName()))
    );

    auto* appearanceNameEdit = new QLineEdit(QString::fromStdString(pin->getAppearanceName()));
    appearanceNameEdit->setStyleSheet(STYLESHEET_LINEEDIT);

    QObject::connect(appearanceNameEdit, &QLineEdit::editingFinished, [pin, appearanceNameEdit]()
    {
        pin->setAppearanceName(appearanceNameEdit->text().toStdString());
        pin->refreshGraphics();
    });

    simulatorPropertySection->addProperty("Appearance Name", appearanceNameEdit);

    auto* directionCombo = new QComboBox();
    directionCombo->setStyleSheet(STYLESHEET_COMBOBOX);
    directionCombo->addItem("Input");
    directionCombo->addItem("Output");
    directionCombo->addItem("Two-Way");
    directionCombo->setCurrentIndex(static_cast<int>(pin->getIODirection()));

    QObject::connect(directionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [pin](int index)
    {
        pin->setIODirection(static_cast<Pin::IODirection>(index));
    });

    simulatorPropertySection->addProperty("Direction", directionCombo);

    simulatorPropertySection->addProperty(
        "State",
        makeReadOnlyPropertyLabel(stateToDisplayString(pin->getEffectingState()))
    );

    simulatorPropertySection->addProperty(
        "Grid Position",
        makeReadOnlyPropertyLabel(QString("(%1, %2)").arg(pin->getGridPosition().x).arg(pin->getGridPosition().y))
    );
}

void ProjectManager::populateComponentProperties(Component* component)
{
    simulatorPropertySection->addProperty(
        "Type",
        makeReadOnlyPropertyLabel(QString::fromStdString(component->getName()))
    );

    auto* appearanceNameEdit = new QLineEdit(QString::fromStdString(component->getAppearanceName()));
    appearanceNameEdit->setStyleSheet(STYLESHEET_LINEEDIT);

    QObject::connect(appearanceNameEdit, &QLineEdit::editingFinished, [component, appearanceNameEdit]()
    {
        component->setAppearanceName(appearanceNameEdit->text().toStdString());

        if (component->getGraphicsObject())
            component->getGraphicsObject()->update();
    });

    simulatorPropertySection->addProperty("Appearance Name", appearanceNameEdit);

    auto* rotationCombo = new QComboBox();
    rotationCombo->setStyleSheet(STYLESHEET_COMBOBOX);
    rotationCombo->addItem(QString::fromUtf8("0\u00B0"));
    rotationCombo->addItem(QString::fromUtf8("90\u00B0"));
    rotationCombo->addItem(QString::fromUtf8("180\u00B0"));
    rotationCombo->addItem(QString::fromUtf8("270\u00B0"));
    rotationCombo->setCurrentIndex(component->getRotation() / 90);

    QObject::connect(rotationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [component](int index)
    {
        if (globalProjectManager->workspace)
            globalProjectManager->workspace->setComponentRotation(component, index * 90);
    });

    simulatorPropertySection->addProperty("Rotation", rotationCombo);

    simulatorPropertySection->addProperty(
        "Grid Position",
        makeReadOnlyPropertyLabel(QString("(%1, %2)").arg(component->getGridPosition().x).arg(component->getGridPosition().y))
    );
}

void ProjectManager::populateWireProperties(Wire* wire)
{
    simulatorPropertySection->addProperty(
        "Type",
        makeReadOnlyPropertyLabel("Wire")
    );

    simulatorPropertySection->addProperty(
        "Segments",
        makeReadOnlyPropertyLabel(QString::number(wire->segments.size()))
    );

    simulatorPropertySection->addProperty(
        "State",
        makeReadOnlyPropertyLabel(stateToDisplayString(wire->getEffectingState()))
    );
}