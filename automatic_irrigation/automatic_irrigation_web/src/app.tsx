import { BrowserRouter, Route, Routes } from "react-router-dom";
import { Dashboard } from "./pages/dashboard";

export function App() {
  return (
    <BrowserRouter>
      <Routes>
        <Route index element={<Dashboard />}></Route>
      </Routes>
    </BrowserRouter>
  );
}
