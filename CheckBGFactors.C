
void CheckBGFactors()
{
    TFile * f = new TFile("outputs.root");

    TCanvas * c1 = new TCanvas();
    Int_t peak = 1730;
    for (auto i = 0; i < 51; i++) {
        TH2D * h = (TH2D*)f->Get(Form("room_background_subtracted/source_%02i", i));
        TH1D * p = h->ProjectionX();
        std::cout << h->GetName() << std::endl;

        p->GetXaxis()->SetRangeUser(peak - 20, peak + 20);
        p->Draw("HIST");
        c1->Update();

        std::cout << "Press any key to continue" << std::endl;
        std::cin.get();

        delete h;
        delete p;
    }

    c1->Close();
} // end CheckBGFactors
