
void CheckBGFactors()
{
    TFile * f = new TFile("outputs.root");

    TCanvas * c1 = new TCanvas();
    for (auto i = 0; i < 51; i++) {
        TH2D * h = (TH2D*)f->Get(Form("room_background_subtracted/source_%02i", i));
        TH1D * p = h->ProjectionX();
        std::cout << h->GetName() << std::endl;

        p->GetXaxis()->SetRangeUser(1420, 1500);
        p->Draw("HIST");
        c1->Update();

        std::cout << "Press any key to continue" << std::endl;
        std::cin.get();

        delete h;
        delete p;
    }

    c1->Close();
} // end CheckBGFactors
