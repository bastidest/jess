#pragma once

#include "ChunkedJournal.hpp"
#include "MainFrame.hpp"
#include "Modeline.hpp"
#include "NcTerminal.hpp"
#include "SdJournal.hpp"

namespace jess
{

class JessMain
{
  NcTerminal m_rootTerminal{};
  NcWindow m_rootWindow = m_rootTerminal.rootWindow();
  Modeline m_modeline{ m_rootWindow };
  MainFrame m_mainFrame{ m_rootWindow };
  std::string m_currentCursor{};
  bool m_bModelineActive{};
  std::span<SdLine> m_currentLines{};
  ChunkedJournal<SdJournal> m_journal{ 1024, 0 };

public:
  KeyCombination getNextKey() { return m_modeline.getKeyCombination().value(); }

  void scrollToBof()
  {
    m_journal.seekToBof();
    redrawTranslation();
  }

  void scrollToEof()
  {
    m_journal.seekToEof();
    redrawTranslation();
  }

  void scrollUpLine()
  {
    m_journal.seekLines( -1 );
    redrawTranslation();
  }

  void scrollDownLine()
  {
    m_journal.seekLines( 1 );
    redrawTranslation();
  }

  void scrollUpPage()
  {
    m_journal.seekLines( -static_cast<int64_t>( m_mainFrame.height() ) );
    redrawTranslation();
  }

  void scrollDownPage()
  {
    m_journal.seekLines( static_cast<int64_t>( m_mainFrame.height() ) );
    redrawTranslation();
  }

  void activateModeline()
  {
    m_bModelineActive = true;
    m_modeline.setActive();
    m_bModelineActive = false;
  }

private:
  void redraw()
  {
    m_mainFrame.drawLines( m_currentLines );
    m_modeline.focus();
  }
  void redrawTranslation()
  {
    m_currentCursor = m_journal.getChunkPositionString();
    m_currentLines = m_journal.getLines( m_mainFrame.height() );
    redraw();
    displayOffset();
  }
  void displayOffset() { m_modeline.displayStatusString( "cursor: " + m_currentCursor ); }
};

}// namespace jess