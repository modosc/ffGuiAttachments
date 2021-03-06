/*
 ==============================================================================

 Copyright (c) 2016, Daniel Walz
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 3. Neither the name of the copyright holder nor the names of its contributors
 may be used to endorse or promote products derived from this software without
 specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ==============================================================================
 */

/*
  ==============================================================================

    ValueTreeTextEditorAttachment.h
    Created: 4 Jun 2016 11:17:03am
    Author:  Oli Larkin / Daniel Walz

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

/**
 \class ValueTreeTextEditorAttachment
 \brief Connects a TextEditor to a ValueTree node to synchronise
 */
class ValueTreeTextEditorAttachment : public juce::TextEditor::Listener,
                                      public juce::ValueTree::Listener {
public:
  /**
   Creates an attachment to synchronise a TextEditor to a ValueTree node.
   You can set a \param property, in which the text the TextEditor is showing is
   stored
   */
  ValueTreeTextEditorAttachment(
      juce::ValueTree &_tree, juce::TextEditor *_textEditor,
      juce::Identifier _property, juce::UndoManager *_undoMgr = nullptr,
      std::function<juce::var(juce::var)> _convertTreeToTextEditor = nullptr,
      std::function<juce::var(juce::var)> _convertTextEditorToTree = nullptr)
      : tree(_tree),
        property(_property),
        undoMgr(_undoMgr),
        updating(false),
        convertTreeToTextEditor(_convertTreeToTextEditor),
        convertTextEditorToTree(_convertTextEditorToTree) {
    // Don't attach an invalid valuetree!
    jassert(tree.isValid());
    textEditor = _textEditor;

    if (tree.hasProperty(property)) {
      auto value = tree.getProperty(property);
      if (convertTreeToTextEditor != nullptr)
        value = convertTreeToTextEditor(value);

      textEditor->setText(value, juce::dontSendNotification);
    } else {
      auto value = textEditor->getText();
      if (convertTextEditorToTree != nullptr)
        value = convertTextEditorToTree(value);

      tree.setProperty(property, value, undoMgr);
    }

    tree.addListener(this);
    textEditor->addListener(this);
  }

  ~ValueTreeTextEditorAttachment() override {
    tree.removeListener(this);
    if (textEditor) {
      textEditor->removeListener(this);
    }
  }

  /**
   This updates the ValueTree to the TextEditor's text
   */
  void textEditorTextChanged(juce::TextEditor &_textEditor) override {
    if (!updating) {
      updating = true;
      if (textEditor == &_textEditor) {
        auto value = textEditor->getText();
        if (convertTextEditorToTree != nullptr)
          value = convertTextEditorToTree(value);

        tree.setProperty(property, value, undoMgr);
      }
      updating = false;
    }
  }

  /**
   This updates the TextEditor to display the ValueTree's property
   */
  void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                const juce::Identifier &_property) override {
    if (!updating) {
      updating = true;
      if (treeWhosePropertyHasChanged == tree && textEditor) {
        if (_property == property) {
          auto value = tree.getProperty(property);
          if (convertTreeToTextEditor != nullptr)
            value = convertTreeToTextEditor(value);

          textEditor->setText(value, juce::dontSendNotification);
        }
      }
      updating = false;
    }
  }

private:
  juce::ValueTree tree;
  juce::Component::SafePointer<juce::TextEditor> textEditor;
  juce::Identifier property;
  juce::UndoManager *undoMgr;
  bool updating;
  std::function<juce::var(juce::var)> convertTreeToTextEditor;
  std::function<juce::var(juce::var)> convertTextEditorToTree;
};
