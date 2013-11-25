/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "SpikeDetectorEditor.h"
#include "SpikeDisplayEditor.h"
#include "../Visualization/SpikeDetectCanvas.h"
#include "../SpikeDetector.h"
#include "ChannelSelector.h"
#include "../../UI/EditorViewport.h"
#include "../AdvancerNode.h"
#include <stdio.h>



SpikeDetectorEditor::SpikeDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : VisualizerEditor(parentNode, useDefaultParameterEditors), isPlural(true),spikeDetectorCanvas(nullptr)

{
	tabText = "Spike Detector";
	

    MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    Typeface::Ptr typeface = new CustomTypeface(mis);
    font = Font(typeface);

    desiredWidth = 300;

    electrodeTypes = new ComboBox("Electrode Types");

    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    for (int i = 0; i < processor->electrodeTypes.size(); i++)
    {
        String type = processor->electrodeTypes[i];
        electrodeTypes->addItem(type += "s", i+1);
    }

	 advancerList = new ComboBox("Advancers");
    advancerList->addListener(this);
    advancerList->setBounds(10,100,150,20);
    addAndMakeVisible(advancerList);
	 updateAdvancerList();

    electrodeTypes->setEditableText(false);
    electrodeTypes->setJustificationType(Justification::centredLeft);
    electrodeTypes->addListener(this);
    electrodeTypes->setBounds(65,30,110,20);
    electrodeTypes->setSelectedId(1);
    addAndMakeVisible(electrodeTypes);

    electrodeList = new ComboBox("Electrode List");
    electrodeList->setEditableText(false);
    electrodeList->setJustificationType(Justification::centredLeft);
    electrodeList->addListener(this);
    electrodeList->setBounds(30,60,115,20);
    addAndMakeVisible(electrodeList);

    numElectrodes = new Label("Number of Electrodes","1");
    numElectrodes->setEditable(true);
    numElectrodes->addListener(this);
    numElectrodes->setBounds(30,30,25,20);
    //labelTextChanged(numElectrodes);
    addAndMakeVisible(numElectrodes);

    upButton = new TriangleButton(1);
    upButton->addListener(this);
    upButton->setBounds(50,30,10,8);
    addAndMakeVisible(upButton);

    downButton = new TriangleButton(2);
    downButton->addListener(this);
    downButton->setBounds(50,40,10,8);
    addAndMakeVisible(downButton);

    plusButton = new UtilityButton("+", titleFont);
    plusButton->addListener(this);
    plusButton->setRadius(3.0f);
    plusButton->setBounds(15,32,14,14);
    addAndMakeVisible(plusButton);


	removeElectrodeButton = new UtilityButton("-",font);
    removeElectrodeButton->addListener(this);
    removeElectrodeButton->setBounds(10,62,14,14);
    addAndMakeVisible(removeElectrodeButton);
    
    thresholdSlider = new ThresholdSlider(font);
    thresholdSlider->setBounds(200,35,75,75);
    addAndMakeVisible(thresholdSlider);
    thresholdSlider->addListener(this);
    thresholdSlider->setActive(false);
    Array<double> v;
    thresholdSlider->setValues(v);

    thresholdLabel = new Label("Name","Threshold");
    font.setHeight(10);
    thresholdLabel->setFont(font);
    thresholdLabel->setBounds(202, 105, 95, 15);
    thresholdLabel->setColour(Label::textColourId, Colours::grey);
    addAndMakeVisible(thresholdLabel);

    // create a custom channel selector
    deleteAndZero(channelSelector);

    channelSelector = new ChannelSelector(false, font);
    addChildComponent(channelSelector);
    channelSelector->setVisible(false);

    
	channelSelector->activateButtons();
	channelSelector->setRadioStatus(true);
    channelSelector->paramButtonsToggledByDefault(false);
   
}

/*void SpikeDetectorEditor::addSpikeToBuffer(SpikeObject so) {


}*/

Visualizer* SpikeDetectorEditor::createNewCanvas()
{

    SpikeDetector* processor = (SpikeDetector*) getProcessor();
    spikeDetectorCanvas = new SpikeDetectCanvas(processor);
	return spikeDetectorCanvas;
}


SpikeDetectorEditor::~SpikeDetectorEditor()
{

    for (int i = 0; i < electrodeButtons.size(); i++)
    {
        removeChildComponent(electrodeButtons[i]);
    }

    deleteAllChildren();

}

void SpikeDetectorEditor::sliderEvent(Slider* slider)
{
    int electrodeNum = -1;

    for (int i = 0; i < electrodeButtons.size(); i++)
    {
        if (electrodeButtons[i]->getToggleState())
        {
            electrodeNum = i; //electrodeButtons[i]->getChannelNum()-1;
            break;
        }
    }

    //   std::cout << "Slider value changed." << std::endl;
    if (electrodeNum > -1)
    {
        SpikeDetector* processor = (SpikeDetector*) getProcessor();
        processor->setChannelThreshold(electrodeList->getSelectedItemIndex(),
                                       electrodeNum,
                                       slider->getValue());
    }

}


void SpikeDetectorEditor::buttonEvent(Button* button)
{
	VisualizerEditor::buttonEvent(button);

    if (electrodeButtons.contains((cElectrodeButton*) button))
    {
	
		
       // if (electrodeEditorButtons[0]->getToggleState()) // EDIT is active
        {
			for (int k=0;k<electrodeButtons.size();k++)
			{
				if (electrodeButtons[k] != button)
					electrodeButtons[k]->setToggleState(false,dontSendNotification);
			}
			if (electrodeButtons.size() == 1)
				electrodeButtons[0]->setToggleState(true,dontSendNotification);

            cElectrodeButton* eb = (cElectrodeButton*) button;
            int channelNum = eb->getChannelNum()-1;

            std::cout << "Channel number: " << channelNum << std::endl;
            Array<int> a;
            a.add(channelNum);
            channelSelector->setActiveChannels(a);

            SpikeDetector* processor = (SpikeDetector*) getProcessor();

            thresholdSlider->setActive(true);
            thresholdSlider->setValue(processor->getChannelThreshold(electrodeList->getSelectedItemIndex(),
                                                                     electrodeButtons.indexOf((cElectrodeButton*) button)));
        }
    }


    int num = numElectrodes->getText().getIntValue();

    if (button == upButton)
    {
        numElectrodes->setText(String(++num), sendNotification);

        return;

    }
    else if (button == downButton)
    {

        if (num > 1)
            numElectrodes->setText(String(--num), sendNotification);

        return;

    }
    else if (button == plusButton)
    {
        // std::cout << "Plus button pressed!" << std::endl;
		updateAdvancerList();
        int type = electrodeTypes->getSelectedId();
        std::cout << type << std::endl;
        int nChans;

        switch (type)
        {
            case 1:
                nChans = 1;
                break;
            case 2:
                nChans = 2;
                break;
            case 3:
                nChans = 4;
                break;
            default:
                nChans = 1;
        }

        for (int n = 0; n < num; n++)
        {
            if (!addElectrode(nChans))
            {
                sendActionMessage("Not enough channels to add electrode.");
            }
        }


        getEditorViewport()->makeEditorVisible(this, true, true);
        return;

    }
    /*else if (1)// if (button == electrodeEditorButtons[0])   // EDIT
    {

        Array<int> activeChannels;

        for (int i = 0; i < electrodeButtons.size(); i++)
        {
            if (button->getToggleState())
            {
                electrodeButtons[i]->setToggleState(false, false);
                electrodeButtons[i]->setRadioGroupId(299);
                channelSelector->activateButtons();
                channelSelector->setRadioStatus(true);
            }
            else
            {
                electrodeButtons[i]->setToggleState(true, false);
                electrodeButtons[i]->setRadioGroupId(0);
                channelSelector->inactivateButtons();
                channelSelector->setRadioStatus(false);
				activeChannels.clear();
                activeChannels.add(electrodeButtons[i]->getChannelNum()-1);
            }
        }

		*/
	/*
        if (!button->getToggleState())
        {
            thresholdSlider->setActive(false);

            // This will be -1 with nothing selected
            int selectedItemIndex = electrodeList->getSelectedItemIndex();
            if (selectedItemIndex != -1)
            {
                drawElectrodeButtons(selectedItemIndex);
            }
            else
            {
                electrodeButtons.clear();
            }
        }*/

        //   channelSelector->setActiveChannels(activeChannels);
	/*
        return;

    }*/
    else if (button == removeElectrodeButton)   // DELETE
    {

        removeElectrode(electrodeList->getSelectedItemIndex());

        getEditorViewport()->makeEditorVisible(this, true, true);

        return;
    }



}

void SpikeDetectorEditor::setThresholdValue(int channel, double threshold)
{
	thresholdSlider->setActive(true);
	thresholdSlider->setValue(threshold);
	repaint();
}

void SpikeDetectorEditor::channelChanged(int chan)
{
    //std::cout << "New channel: " << chan << std::endl;
	if (chan == -1)
		return;
    for (int i = 0; i < electrodeButtons.size(); i++)
    {
        if (electrodeButtons[i]->getToggleState())
        {
            electrodeButtons[i]->setChannelNum(chan);
            electrodeButtons[i]->repaint();
			Array<int> a;
			a.add(chan-1);
			channelSelector->setActiveChannels(a);
            SpikeDetector* processor = (SpikeDetector*) getProcessor();
            processor->setChannel(electrodeList->getSelectedItemIndex(),
                                  i,
                                  chan-1);
        }
    }

}

void SpikeDetectorEditor::refreshElectrodeList()
{
    electrodeList->clear();

    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    StringArray electrodeNames = processor->getElectrodeNames();

    for (int i = 0; i < electrodeNames.size(); i++)
    {
        electrodeList->addItem(electrodeNames[i], electrodeList->getNumItems()+1);
    }

    if (electrodeList->getNumItems() > 0)
    {
        electrodeList->setSelectedId(electrodeList->getNumItems(), true);
        electrodeList->setText(electrodeList->getItemText(electrodeList->getNumItems()-1));
        lastId = electrodeList->getNumItems();
        electrodeList->setEditableText(true);

        drawElectrodeButtons(electrodeList->getNumItems()-1);
    }
	if (spikeDetectorCanvas != nullptr)
		spikeDetectorCanvas->update();
}

bool SpikeDetectorEditor::addElectrode(int nChans)
{
    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    if (processor->addElectrode(nChans))
    {
        refreshElectrodeList();
        return true;
    }
    else
    {
        return false;
    }

}


void SpikeDetectorEditor::removeElectrode(int index)
{
    std::cout << "Deleting electrode number " << index << std::endl;
    SpikeDetector* processor = (SpikeDetector*) getProcessor();
    processor->removeElectrode(index);
    refreshElectrodeList();

    int newIndex = jmin(index, electrodeList->getNumItems()-1);
    newIndex = jmax(newIndex, 0);

    electrodeList->setSelectedId(newIndex, true);
    electrodeList->setText(electrodeList->getItemText(newIndex));

    if (electrodeList->getNumItems() == 0)
    {
        electrodeButtons.clear();
        electrodeList->setEditableText(false);
    }
}

void SpikeDetectorEditor::labelTextChanged(Label* label)
{
    if (label->getText().equalsIgnoreCase("1") && isPlural)
    {
        for (int n = 1; n < electrodeTypes->getNumItems()+1; n++)
        {
            electrodeTypes->changeItemText(n,
                                           electrodeTypes->getItemText(n-1).trimCharactersAtEnd("s"));
        }

        isPlural = false;

        String currentText = electrodeTypes->getText();
        electrodeTypes->setText(currentText.trimCharactersAtEnd("s"));

    }
    else if (!label->getText().equalsIgnoreCase("1") && !isPlural)
    {
        for (int n = 1; n < electrodeTypes->getNumItems()+1; n++)
        {
            String currentString = electrodeTypes->getItemText(n-1);
            currentString += "s";

            electrodeTypes->changeItemText(n,currentString);
        }
        isPlural = true;

        String currentText = electrodeTypes->getText();
        electrodeTypes->setText(currentText += "s");
    }

    getEditorViewport()->makeEditorVisible(this, false, true);

}

void SpikeDetectorEditor::comboBoxChanged(ComboBox* comboBox)
{
    if (comboBox == electrodeList)
    {
		updateAdvancerList();

        int ID = comboBox->getSelectedId();

        if (ID == 0)
        {
            SpikeDetector* processor = (SpikeDetector*) getProcessor();

            processor->setElectrodeName(lastId, comboBox->getText());
            refreshElectrodeList();

        }
		else
        {
			SpikeDetector* processor = (SpikeDetector*) getProcessor();
            lastId = ID;
			processor->setCurrentElectrodeIndex(ID-1);
            drawElectrodeButtons(ID-1);

        }
	} else if ( comboBox == advancerList)
	{
		// attach advancer to electrode.
		int electrodeIndex = electrodeList->getSelectedId()-1;
		SpikeDetector* processor = (SpikeDetector*) getProcessor();
		processor->setElectrodeAdvancer(electrodeIndex,advancerNames[advancerList->getSelectedId()-1]);
	}
	
}

void SpikeDetectorEditor::checkSettings()
{
    electrodeList->setSelectedItemIndex(0);
}

void SpikeDetectorEditor::drawElectrodeButtons(int ID)
{

    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    electrodeButtons.clear();

    int width = 20;
    int height = 15;

    int numChannels = processor->getNumChannels(ID);
    int row = 0;
    int column = 0;

    Array<int> activeChannels;
    Array<double> thresholds;

    for (int i = 0; i < numChannels; i++)
    {
        cElectrodeButton* button = new cElectrodeButton(processor->getChannel(ID,i)+1);
        electrodeButtons.add(button);

        

		if (i== 0) {
			activeChannels.add(processor->getChannel(ID,i));
			thresholds.add(processor->getChannelThreshold(ID,i));
		}

		button->setToggleState(i == 0, false);

		button->setBounds(155+(column++)*width, 60+row*height, width, 15);

		addAndMakeVisible(button);
        button->addListener(this);

        if (column%2 == 0)
        {
            column = 0;
            row++;
        }

    }

    channelSelector->setActiveChannels(activeChannels);
	
    thresholdSlider->setValues(thresholds);
	thresholdSlider->setActive(true);
	thresholdSlider->setEnabled(true);
	thresholdSlider->setValue(processor->getChannelThreshold(ID,0),dontSendNotification);
	repaint();
	if (spikeDetectorCanvas!=nullptr)
		spikeDetectorCanvas->update();
}




void cElectrodeButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState() == true)
        g.setColour(Colours::orange);
    else
        g.setColour(Colours::darkgrey);

    if (isMouseOver)
        g.setColour(Colours::white);

    g.fillRect(0,0,getWidth(),getHeight());

    // g.setFont(buttonFont);
    g.setColour(Colours::black);

    g.drawRect(0,0,getWidth(),getHeight(),1.0);

    if (chan >= 0)
        g.drawText(String(chan),0,0,getWidth(),getHeight(),Justification::centred,true);
}


void ElectrodeEditorButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState() == true)
        g.setColour(Colours::darkgrey);
    else
        g.setColour(Colours::lightgrey);

    g.setFont(font);

    g.drawText(name,0,0,getWidth(),getHeight(),Justification::left,true);
}


ThresholdSlider::ThresholdSlider(Font f) : Slider("name"), font(f)
{

    setSliderStyle(Slider::Rotary);
    setRange(-400,400.0f,10.0f);
    setValue(-20.0f);
    setTextBoxStyle(Slider::NoTextBox, false, 40, 20);

}

void ThresholdSlider::paint(Graphics& g)
{

    ColourGradient grad = ColourGradient(Colour(40, 40, 40), 0.0f, 0.0f,
                                         Colour(80, 80, 80), 0.0, 40.0f, false);

    Path p;
    p.addPieSegment(3, 3, getWidth()-6, getHeight()-6, 5*double_Pi/4-0.2, 5*double_Pi/4+3*double_Pi/2+0.2, 0.5);

    g.setGradientFill(grad);
    g.fillPath(p);

    String valueString;

    if (isActive)
    {
        p = makeRotaryPath(getMinimum(), getMaximum(), getValue());
        g.setColour(Colour(240,179,12));
        g.fillPath(p);

        valueString = String((int) getValue());
    }
    else
    {

        valueString = "";

        for (int i = 0; i < valueArray.size(); i++)
        {
            p = makeRotaryPath(getMinimum(), getMaximum(), valueArray[i]);
            g.setColour(Colours::lightgrey.withAlpha(0.4f));
            g.fillPath(p);
            valueString = String((int) valueArray.getLast());
        }

    }

    font.setHeight(9.0);
    g.setFont(font);
    int stringWidth = font.getStringWidth(valueString);

    g.setFont(font);

    g.setColour(Colours::darkgrey);
    g.drawSingleLineText(valueString, getWidth()/2 - stringWidth/2, getHeight()/2+3);

}

Path ThresholdSlider::makeRotaryPath(double min, double max, double val)
{

    Path p;

    double start = 5*double_Pi/4 - 0.11;

    double range = (val-min)/(max - min)*1.5*double_Pi + start + 0.22;

    p.addPieSegment(6,6, getWidth()-12, getHeight()-12, start, range, 0.65);

    return p;

}

void ThresholdSlider::setActive(bool t)
{
    isActive = t;
    repaint();
}

void ThresholdSlider::setValues(Array<double> v)
{
    valueArray = v;
}



void SpikeDetectorEditor::updateAdvancerList()
{
	
	ProcessorGraph *g = getProcessor()->getProcessorGraph();
	Array<GenericProcessor*> p = g->getListOfProcessors();
	for (int k=0;k<p.size();k++)
	{
		if (p[k]->getName() == "Advancers")
		{
			AdvancerNode *node = (AdvancerNode *)p[k];
			advancerNames = node->getAdvancerNames();
			advancerList->clear();
			for (int i=0;i<advancerNames.size();i++)
			{
				advancerList->addItem(advancerNames[i],1+i);
			}
		}
	}
	repaint();
}
