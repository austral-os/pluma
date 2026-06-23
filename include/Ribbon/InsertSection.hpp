#pragma once

#include <horizon/RibbonToolbar.hpp>
#include <horizon/RibbonButton.hpp>
#include <memory>

namespace pluma_app {

class InsertSection {
public:
  InsertSection(horizon::RibbonToolbar *ribbon, int tab_index);

  horizon::RibbonButton* btn_page_break() const { return m_btn_page_break; }
  horizon::RibbonButton* btn_blank_page() const { return m_btn_blank_page; }
  horizon::RibbonButton* btn_line_break() const { return m_btn_line_break; }

  horizon::RibbonButton* btn_table() const { return m_btn_table; }
  horizon::RibbonButton* btn_image() const { return m_btn_image; }

  horizon::RibbonButton* btn_hyperlink() const { return m_btn_hyperlink; }
  horizon::RibbonButton* btn_bookmark() const { return m_btn_bookmark; }
  horizon::RibbonButton* btn_cross_ref() const { return m_btn_cross_ref; }

  horizon::RibbonButton* btn_field() const { return m_btn_field; }

private:
  horizon::RibbonSection *m_section_page = nullptr;
  horizon::RibbonSection *m_section_table = nullptr;
  horizon::RibbonSection *m_section_image = nullptr;
  horizon::RibbonSection *m_section_links = nullptr;
  horizon::RibbonSection *m_section_fields = nullptr;

  horizon::RibbonButton *m_btn_page_break = nullptr;
  horizon::RibbonButton *m_btn_blank_page = nullptr;
  horizon::RibbonButton *m_btn_line_break = nullptr;

  horizon::RibbonButton *m_btn_table = nullptr;
  horizon::RibbonButton *m_btn_image = nullptr;

  horizon::RibbonButton *m_btn_hyperlink = nullptr;
  horizon::RibbonButton *m_btn_bookmark = nullptr;
  horizon::RibbonButton *m_btn_cross_ref = nullptr;

  horizon::RibbonButton *m_btn_field = nullptr;
};

} // namespace pluma_app
