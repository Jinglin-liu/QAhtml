#include "DrawTimingOkay.h"

#include <qahtml/QADrawClient.h>
#include <qahtml/QADrawDB.h>

#include <phool/phool.h>

#include <TCanvas.h>
#include <TH1.h>
#include <TLine.h>
#include <TPad.h>
#include <TText.h>

#include <iostream>
#include <format>
#include <map>
#include <set>
#include <sstream>
#include <boost/format.hpp>

DrawTimingOkay::DrawTimingOkay (
	std::string const& name
) : SingleCanvasDrawer(name) {
	// Do nothing
}

int
DrawTimingOkay::MakeCanvas (
	int width,
	int height
) {
	if (SingleCanvasDrawer::MakeCanvas(width, height)) return 0;

	m_canvas->cd();
	m_title_pad = new TPad (
		(boost::format("%s_title_pad") % m_name).str().c_str(),
		(boost::format("%s_title_pad") % m_name).str().c_str(),
		0.0, 0.9, 1.0, 1.0
	);
	m_title_pad->SetFillStyle(4000);
	m_title_pad->Range(0.0, 0.0, 1.0, 1.0);
	m_title_pad->Draw();

	m_canvas->cd();
	m_text_pad = new TPad (
		(boost::format("%s_text_pad") % m_name).str().c_str(),
		(boost::format("%s_text_pad") % m_name).str().c_str(),
		0.0, 0.0, 1.0, 0.9
	);
	m_text_pad->SetFillStyle(4000);
	m_text_pad->Range(0.0, 0.0, 1.0, 1.0);
	m_text_pad->Draw();

	return 0;
}

int
DrawTimingOkay::DrawCanvas (
) {
	MakeCanvas();
	m_canvas->SetEditable(true);

	QADrawClient *cl = QADrawClient::instance();

	m_title_pad->cd();
	TText title_text;
	title_text.SetTextAlign(22);
	title_text.SetTextSize(0.5);
	title_text.DrawText(0.5, 0.5, (boost::format("INTT Timing, Run %08d") % cl->RunNumber()).str().c_str());

	// Some of the original productions were for triggered only
	// instead of throwing some kind of error or early return,
	// assume it is triggered if the hist isn't in the file
	bool streaming = false;
	std::string is_streaming_hist_name = (boost::format("%s_is_streaming") % m_prefix).str();
	auto* is_streaming_hist = dynamic_cast<TH1*>(cl->getHisto(is_streaming_hist_name.c_str()));
	if (is_streaming_hist) { streaming = (is_streaming_hist->GetBinContent(1) == 1); }

	// For each felix server, get the distribution of peaks over felix channels
	std::array<std::map<int, int>, 8> server_peak_counts;
	std::array<int, 8> masked_felix_channels{};
	for (int felix_server = 0; felix_server < 8; ++felix_server) {
		for (int felix_channel = 0; felix_channel < 14; ++felix_channel) {
			std::string hist_name = (boost::format("%s_bco_distribution_server%01d_channel%02d") % m_prefix % felix_server % felix_channel).str();
			auto* hist = dynamic_cast<TH1*>(cl->getHisto(hist_name.c_str()));
			if (!hist) {
				std::cout
					<< PHWHERE
					<< "Could not get hist '" << hist_name << "' from client"
					<< std::endl;
				return 1;
			}

			// Do not consider felix channels which are completely masked
			bool masked = (hist->GetEntries() == 0);
			if (masked) { ++masked_felix_channels[felix_server]; }
			int peak_bin = masked ? -1 : hist->GetBinCenter(hist->GetMaximumBin());
			++server_peak_counts[felix_server][peak_bin];
		}
	}

	// They are sorted by peak position--sort them by count
	// (useful for selected the mode of the distribution, and printing)
	std::array<std::map<int, int>, 8> sorted_server_peak_counts;
	for (int felix_server = 0; felix_server < 8; ++felix_server) {
		for (auto const& [peak, count] : server_peak_counts[felix_server]) {
			sorted_server_peak_counts[felix_server][count] = peak;
		}
	}

	// Now for the whole of the intt, get the distribution of peaks over felix servers
	// For each felix server, choose the mode of felix channel peaks as the server peak
	std::set<int> intt_peaks;
	for (auto const& peak_map : sorted_server_peak_counts) {
		// We are interested in the first unmasked channel we find sorting by count
		// the for loop makes this control flow syntactically easier to implement
		for (auto itr = peak_map.rbegin(); itr != peak_map.rend(); ++itr) {
			// Ignore masked/empty felix channels
			if (itr->second == -1) { continue; }
			intt_peaks.insert(itr->second);
			break;
		}
	}

	// The overall timing check is that there is that the modal peak of all felix servers is the same
	bool timing_okay = intt_peaks.size() == 1;
	std::array<bool, 8> timing_okay_by_channel{};
	for (int felix_server = 0; felix_server < 8; ++felix_server) {
		// The internal check is that enough felix channels share a common peak position
		auto const& peak_map = sorted_server_peak_counts[felix_server];
		int num_masked = masked_felix_channels[felix_server];
		// We are interested in the first unmasked channel we find sorting by count
		// the for loop makes this control flow syntactically easier to implement
		for (auto itr = peak_map.rbegin(); itr != peak_map.rend(); ++itr) {
			// Ignore masked/empty channels, subtract the number of masked channels from the requirement
			if (itr->second == -1) { continue; }
			timing_okay_by_channel[felix_server] = !(itr->first < m_min_timed_channels - num_masked);
			// In the streaming case, require the server peaks occur at 24
			if (streaming) { timing_okay_by_channel[felix_server] = timing_okay_by_channel[felix_server] && (itr->second == 24); }
			timing_okay = (timing_okay && timing_okay_by_channel[felix_server]);
			break;
		}
	}

	m_text_pad->cd();
	TText okay_text;
	okay_text.SetTextAlign(22);
	okay_text.SetTextSize(0.1);
	if (timing_okay) {
		okay_text.SetTextColor(kGreen+1);
		okay_text.DrawText(0.5, 0.9, "Timing Okay");
	} else {
		okay_text.SetTextColor(kRed+1);
		okay_text.DrawText(0.5, 0.9, "Timing Not Okay");
	}

	okay_text.SetTextAlign(12);
	okay_text.SetTextSize(0.05);
	okay_text.SetTextColor(kBlack);
	for (int felix_server = 0; felix_server < 8; ++felix_server) {
		auto const& peak_map = sorted_server_peak_counts[felix_server];
		std::stringstream peak_stream;
		peak_stream
			<< (boost::format("intt%01d peak (counts): ") % felix_server).str();
		for (auto itr = peak_map.rbegin();;) {
			peak_stream
				<< ((itr->second == -1) ? std::string{"masked"} : std::to_string(itr->second))
				<< " (" << itr->first << ")";
			if (++itr == peak_map.rend()) break;
			peak_stream
				<< ", ";
		}
		okay_text.SetTextColor(timing_okay_by_channel[felix_server] ? kGreen+1 : kRed+1);
		okay_text.DrawText(0.25, 0.75 - (felix_server + 0.5) / 16, peak_stream.str().c_str());
	}

	m_canvas->Update();
	m_canvas->Show();
	m_canvas->SetEditable(false);

	return 0;
}
